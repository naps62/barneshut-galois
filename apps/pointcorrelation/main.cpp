// C++ includes
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
using std::string;
#include <vector>
using std::vector;

// C includes
#include <stdlib.h>
#include <string.h>
#include <time.h>

// library includes
#include <llvm/Support/CommandLine.h>
#include <Lonestar/BoilerPlate.h>
#include <Galois/Galois.h>
#include <Galois/Statistic.h>
#include <CGAL/spatial_sort.h>
#include <papi.h>

// local includes
#include "cgal.h"
#include "point.h"
#include "kdtree.h"
#include "utilities.h"





const char* name = "Two-Point Correlation";
const char* desc = "Counting the pairs of points which lie in a given radius.";
const char* url = "pointcorrelation";

static llvm::cl::opt<unsigned> npoints("n", llvm::cl::desc("Number of points"), llvm::cl::init(32));
static llvm::cl::opt<double> radius("r", llvm::cl::desc("Threshold radius"), llvm::cl::init(3));
static llvm::cl::opt<unsigned> seed("seed", llvm::cl::desc("Pseudo-random number generator seed. Defaults to current timestamp"), llvm::cl::init(time(NULL)));
static llvm::cl::opt<unsigned> blocksize("bs", llvm::cl::desc("Block size (number of points to use in a block). Low values mean excessive number of instructions. High values exceed cache capacity."), llvm::cl::init(0));
static llvm::cl::opt<bool> togglesort("sort", llvm::cl::desc("Toggle spatial sort."), llvm::cl::init(false));
static llvm::cl::opt<bool> g("g", llvm::cl::desc("Toggle Galois."), llvm::cl::init(false));
static llvm::cl::opt<string> papicn("papi", llvm::cl::desc("PAPI counter name."), llvm::cl::init(string("")));

#define DIM 3

int main (int argc, char *argv[]) {
	LonestarStart(argc, argv, name, desc, url);

	Point<DIM>::Block points;

	std::cerr << "Using " << npoints << " points." << std::endl;
	generateInput(points, npoints, seed);

	//	Sort points
	if (togglesort) {
		std::cerr << "* Using sorted input." << std::endl;
		CGAL::spatial_sort(points.begin(), points.end(), PointSpatialSortingTraits());
	}

	KdTree<DIM> tree(points);

	//
	unsigned result;//<	Final result.
	Galois::StatTimer tAlgorithm;
	double tTraversalAvg;

	if (numThreads > 1)
		g = true;
	// const bool g = numThreads > 0;//	activate Galois
	const bool b = blocksize > 0;//	activate blocks

	typedef Point<DIM>::Block Block;
	vector<Block> blocks;

	//	Prepare PAPI
	long long int value;
	if (!papicn.empty()) {
		std::cerr << "* Using PAPI to measure counter [" << papicn << ']' << std::endl;
#ifndef NDEBUG
		assert(PAPI_library_init(PAPI_VER_CURRENT) == PAPI_VER_CURRENT);
		assert(PAPI_thread_init(getTID) == PAPI_OK);
#else
		PAPI_library_init(PAPI_VER_CURRENT);
		PAPI_thread_init(getTID);
#endif
	}

	//	Split into blocks
	if (b) {
		std::cerr << "* Using point blocking." << std::endl;
		blocks = Point<DIM>::blocks(points, blocksize);
	}

	//	Prepare Galois
	if (g) {
		std::cerr << "* Using parallel implementation with Galois." << std::endl;
		count.reset(0);
	} else
		std::cerr << "* Using sequential implementation." << std::endl;

	//	two point correlation
	if (g && b) {
		//	Parallel (with Galois) and blocked
		Galois::GAccumulator<unsigned long> tTraversalTotal;
		tTraversalTotal.reset(0);
		Galois::GAccumulator<long long int> papiValueTotal;
		papiValueTotal.reset(0);
		KdTree<DIM>::BlockedCorrelator correlator(tree, radius, &tTraversalTotal, papicn, &papiValueTotal);

		tAlgorithm.start();
		Galois::for_each(Point<DIM>::wrap(blocks.begin()), Point<DIM>::wrap(blocks.end()), correlator);
		tAlgorithm.stop();

		//	average traversal time
		tTraversalAvg = (double) tTraversalTotal.get() / (double) points.size();

		//	final correlation result
		result = (count.get() - points.size()) / 2;

		//	PAPI value
		value = papiValueTotal.get();
	} else if (g) {
		//	Parallel (with Galois)
		Galois::GAccumulator<unsigned long> tTraversalTotal;
		tTraversalTotal.reset(0);
		Galois::GAccumulator<long long int> papiValueTotal;
		papiValueTotal.reset(0);
		KdTree<DIM>::Correlator correlator(tree, radius, &tTraversalTotal, papicn, &papiValueTotal);

		tAlgorithm.start();
		Galois::for_each(Point<DIM>::wrap(points.begin()), Point<DIM>::wrap(points.end()), correlator);
		tAlgorithm.stop();

		//	average traversal time
		tTraversalAvg = (double) tTraversalTotal.get() / (double) points.size();

		//	final correlation result
		result = (count.get() - points.size()) / 2;

		//	PAPI value
		value = papiValueTotal.get();
	} else if (b) {
		result = 0;
		tAlgorithm.start();
		for (unsigned i = 0; i < blocks.size(); ++i)
			result += tree.correlated(blocks[i], radius);
		tAlgorithm.stop();
		result = (result - points.size()) / 2;
	} else {
		Galois::StatTimer tTraversal;
		unsigned long tTraversalTotal = 0;

		result = 0;

		if (!papicn.empty()) {
			int eventSet = PAPI_NULL;
			assert(PAPI_create_eventset(&eventSet) == PAPI_OK);

			int event;
			char * name = strdup(papicn.c_str());
			assert(PAPI_event_name_to_code(name, &event) == PAPI_OK);
			free(name);

			assert(PAPI_add_event(eventSet, event) == PAPI_OK);
			assert(PAPI_start(eventSet) == PAPI_OK);

			tAlgorithm.start();
			for (unsigned i = 0; i < points.size(); ++i) {
				tTraversal.start();
				result += tree.correlated(*points[i], radius);
				tTraversal.stop();
				tTraversalTotal += tTraversal.get_usec();
			}
			tAlgorithm.stop();

			assert(PAPI_stop(eventSet, &value) == PAPI_OK);
			assert(PAPI_cleanup_eventset(eventSet) == PAPI_OK);
			assert(PAPI_destroy_eventset(&eventSet) == PAPI_OK);
		} else {
			tAlgorithm.start();
			for (unsigned i = 0; i < points.size(); ++i) {
				tTraversal.start();
				result += tree.correlated(*points[i], radius);
				tTraversal.stop();
				tTraversalTotal += tTraversal.get_usec();
			}
			tAlgorithm.stop();
		}

		tTraversalAvg = (double) tTraversalTotal / (double) points.size();
		result = (result - points.size()) / 2;
	}
	std::cerr << "\t\t" << (double) tAlgorithm.get_usec() * 1e-6 << " seconds" << std::endl;
	std::cerr << "\t\t" << tTraversalAvg * 1e-3 << " miliseconds" << std::endl;
	std::cout << result << std::endl;

	//	Cleanup PAPI
	if (!papicn.empty()) {
		std::cerr << "- " << papicn << ":\t" << value << std::endl;
		PAPI_shutdown();
	}

	//	CLEANUP
	for (unsigned i = 0; i < points.size(); ++i)
		delete points[i];

	return 0;
}
