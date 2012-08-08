/** Barnes-hut application -*- C++ -*-
 * @file
 * @section License
 *
 * Galois, a framework to exploit amorphous data-parallelism in irregular
 * programs.
 *
 * Copyright (C) 2011, The University of Texas at Austin. All rights reserved.
 * UNIVERSITY EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES CONCERNING THIS
 * SOFTWARE AND DOCUMENTATION, INCLUDING ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR ANY PARTICULAR PURPOSE, NON-INFRINGEMENT AND WARRANTIES OF
 * PERFORMANCE, AND ANY WARRANTY THAT MIGHT OTHERWISE ARISE FROM COURSE OF
 * DEALING OR USAGE OF TRADE.  NO WARRANTY IS EITHER EXPRESS OR IMPLIED WITH
 * RESPECT TO THE USE OF THE SOFTWARE OR DOCUMENTATION. Under no circumstances
 * shall University be liable for incidental, special, indirect, direct or
 * consequential damages or loss of profits, interruption of business, or
 * related expenses which may arise from use of Software or Documentation,
 * including but not limited to those resulting from defects in Software and/or
 * Documentation, or loss or inaccuracy of data of any kind.
 *
 * @author Martin Burtscher <burtscher@txstate.edu>
 * @author Donald Nguyen <ddn@cs.utexas.edu>
 */
#include <limits>
#include <iostream>
#include <vector>
#include <stack>
#include <string>
using std::string;

#include <strings.h>
#include <boost/math/constants/constants.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include "Galois/Galois.h"
#include "Galois/Statistic.h"
#include "llvm/Support/CommandLine.h"
#include "Lonestar/BoilerPlate.h"

#include <CGAL/spatial_sort.h>
#include <papi.h>

#include "config.h"





#include "f_BuildOctree.h"
#include "f_ComputeCenterOfMass.h"
	/** Build blocks of bodies */
#include "f_BodyBlocksBuild.h"
	/** Compute Forces was replaced with CleanComputeForces, and later with BlockedComputeForces */
#include "f_CleanComputeForces.h"
#include "f_BlockedComputeForces.h"
#include "f_AdvanceBodies.h"
#include "f_ReduceBoxes.h"
#include "utilities.h"
#include "cgal.h"






static llvm::cl::opt<unsigned> nbodies("n", llvm::cl::desc("Number of bodies"), llvm::cl::init(/*10000*/32));
static llvm::cl::opt<unsigned> ntimesteps("ts", llvm::cl::desc("Number of steps"), llvm::cl::init(1));
static llvm::cl::opt<int> seed("seed", llvm::cl::desc("Random seed"), llvm::cl::init(time(NULL)));
static llvm::cl::opt<bool> use_sort("sort", llvm::cl::desc("Toggle using sorted bodies."), llvm::cl::init(false));
static llvm::cl::opt<bool> print_output("out", llvm::cl::desc("Toggle printing the final result."), llvm::cl::init(false));
static llvm::cl::opt<int> block_size("bs", llvm::cl::desc("Block size"), llvm::cl::init(0));
static llvm::cl::opt<string> papi_event_name("papi", llvm::cl::desc("Name of the PAPI event to measure."), llvm::cl::init(""));



namespace Barneshut {
	const char* name = "Barnshut N-Body Simulator";
	const char* desc =
		"Simulation of the gravitational forces in a galactic cluster using the "
		"Barnes-Hut n-body algorithm\n";
	const char* url = "barneshut";

	Config config;

	void pointBlockInput(Bodies& bodies, BodyBlocks& body_blocks, int block_size) {
		body_blocks.clear();
	}


	template<typename T>
		struct Deref : public std::unary_function<T, T*> {
			T* operator()(T& item) const { return &item; }
		};

	boost::transform_iterator<Deref<Body>, Bodies::iterator> 
		wrap(Bodies::iterator it) {
			return boost::make_transform_iterator(it, Deref<Body>());
		}

	boost::transform_iterator<Deref<BodiesPtr*>, BodyBlocks::iterator>
		wrap(BodyBlocks::iterator it) {
			return boost::make_transform_iterator(it, Deref<BodiesPtr*>());
		}

	void run (int nbodies, int ntimesteps, int seed) {
		Bodies bodies;
		BodyBlocks body_blocks;
		SpatialBodySortingTraits sst;

		generateInput(bodies, nbodies, seed);
		/*for(int i = 0; i < nbodies; ++i)
		  std::cout << "body " << i << " " << bodies[i] << std::endl;*/

		typedef GaloisRuntime::WorkList::dChunkedLIFO<256> WL;

		//  Prepare PAPI
		long long int papi_value = 0;
		if (!papi_event_name.empty()) {
			std::cerr << "* Using PAPI to measure counter [" << papi_event_name << "]." << std::endl;
			assert(PAPI_library_init(PAPI_VER_CURRENT) == PAPI_VER_CURRENT);
			assert(PAPI_thread_init(getTID) == PAPI_OK);
		}

		//	report activated switches
		std::cerr << "* Using parallel implementation (Galois) with " << numThreads << " threads." << std::endl;
		if (use_sort)
			std::cerr << "* Using spatial sorting (bodies)." << std::endl;
		if (block_size > 0)
			std::cerr << "* Using point blocking." << std::endl;

		//
		// Main loop
		//
		Galois::StatTimer tAlgorithm;
		Galois::GAccumulator<unsigned> tTraversalTotal;
		tAlgorithm.start();
		for (int step = 0; step < ntimesteps; step++) {

			// Do tree building sequentially
			Galois::setActiveThreads(1);

			//
			// Step 0.1. Body ordering goes here
			//
			if (use_sort)
				CGAL::spatial_sort(bodies.begin(), bodies.end(), sst);

			//
			// Step 0.2. BodyBlocks build
			//
			if (block_size > 0)
				Galois::for_each<WL>(wrap(bodies.begin()), wrap(bodies.end()),
					BodyBlocksBuild(&body_blocks, block_size));

			//
			// Step 1. Generate a bounding box that contains all points. This is done sequentially
			//
			BoundingBox box;
			ReduceBoxes reduceBoxes(box);
			Galois::for_each<WL>(wrap(bodies.begin()), wrap(bodies.end()),
					ReduceBoxes(box));
			OctreeInternal* top = new OctreeInternal(box.center());

			//
			// Step 2. Build the Octree
			//
			Galois::for_each<WL>(wrap(bodies.begin()), wrap(bodies.end()),
					BuildOctree(top, box.radius()));


			//
			// Step 3. Compute center of mass for each point of the tree
			//
			ComputeCenterOfMass computeCenterOfMass(top);
			computeCenterOfMass();

			// Parallel stuff starts here
			Galois::StatTimer T_parallel("ParallelTime");
			T_parallel.start();
			Galois::setActiveThreads(numThreads);

			//
			// Step 4. Compute forces for each body
			//
			//Galois::for_each<WL>(wrap(bodies.begin()), wrap(bodies.end()),
			//    CleanComputeForces(top, box.diameter()));
			// Galois::for_each<WL>(wrap(body_blocks.begin()), wrap(body_blocks.end()), BlockedComputeForces(top, box.diameter()));
			Galois::GAccumulator<long long int> papi_value_total;
			if (block_size > 0) {
				BlockedComputeForces bcf(top, box.diameter(), config.itolsq, config.dthf, config.epssq, &tTraversalTotal, papi_event_name, &papi_value_total);
				Galois::for_each<WL>(wrap(body_blocks.begin()), wrap(body_blocks.end()), bcf);
			} else {
				CleanComputeForces ccf(top, box.diameter(), config.itolsq, config.dthf, config.epssq, &tTraversalTotal, papi_event_name, &papi_value_total);
				Galois::for_each<WL>(wrap(bodies.begin()), wrap(bodies.end()), ccf);
			}
			papi_value = papi_value_total.get();

			//
			// Step 5. Update body positions
			//
			Galois::for_each<WL>(wrap(bodies.begin()), wrap(bodies.end()),
					AdvanceBodies(config.dthf, config.dtime));
			T_parallel.stop();

			// std::cout 
			// 	<< "Timestep " << step
			// 	<< " Center of Mass = " << top->pos << "\n";
			delete top;
		}
		tAlgorithm.stop();

		if (print_output) {
			std::cout << std::endl << "Final positions:" << std::endl;
			for(int i = 0; i < nbodies; ++i) {
				std::cout << i << ", " << bodies[i].pos << std::endl;
			}
		}

		std::cerr << '\t' << (double) tAlgorithm.get_usec() * 1e-6 << " seconds" << std::endl;
		std::cerr << '\t' << (double) tTraversalTotal.get() * 1e-3 / (nbodies * ntimesteps) << " miliseconds" << std::endl;

		//	Cleanup PAPI
		if (!papi_event_name.empty()) {
			std::cerr << "- " << papi_event_name << ":\t" << papi_value << std::endl;
			PAPI_shutdown();
		}
	}

} // end namespace

int main(int argc, char** argv) {
	Galois::StatManager M;
	LonestarStart(argc, argv, Barneshut::name, Barneshut::desc, Barneshut::url);
	std::cout.setf(std::ios::right|std::ios::scientific|std::ios::showpoint);

	std::cerr << "configuration: "
		<< nbodies << " bodies, "
		<< ntimesteps << " time steps" << std::endl << std::endl;
	// std::cout << "Num. of threads: " << numThreads << std::endl;

	Galois::StatTimer T;
	T.start();
	Barneshut::run(nbodies, ntimesteps, seed);
	T.stop();
}
