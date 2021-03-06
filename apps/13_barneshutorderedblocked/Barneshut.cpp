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
#include <strings.h>
#include <boost/math/constants/constants.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include "Galois/Galois.h"
#include "Galois/Statistic.h"
#include "llvm/Support/CommandLine.h"
#include "Lonestar/BoilerPlate.h"
#include <CGAL/spatial_sort.h>

namespace {
const char* name = "Barnshut N-Body Simulator";
const char* desc =
  "Simulation of the gravitational forces in a galactic cluster using the "
  "Barnes-Hut n-body algorithm\n";
const char* url = "barneshut";

static llvm::cl::opt<int> nbodies("n", llvm::cl::desc("Number of bodies"), llvm::cl::init(/*10000*/32));
static llvm::cl::opt<int> ntimesteps("steps", llvm::cl::desc("Number of steps"), llvm::cl::init(1));
static llvm::cl::opt<int> seed("seed", llvm::cl::desc("Random seed"), llvm::cl::init(7));
static llvm::cl::opt<int> output_final("out", llvm::cl::desc("Output final result"), llvm::cl::init(0));
static llvm::cl::opt<int> block_size("b", llvm::cl::desc("Block Size"), llvm::cl::init(4));

#include "Point.h"
#include "Octree.h"
#include "BoundingBox.h"

struct Config {
  const double dtime; // length of one time step
  const double eps; // potential softening parameter
  const double tol; // tolerance for stopping recursion, <0.57 to bound error
  const double dthf, epssq, itolsq;
  Config() :
    dtime(0.5),
    eps(0.05),
    tol(0.025),
    dthf(dtime * 0.5),
    epssq(eps * eps),
    itolsq(1.0 / (tol * tol))  { }
};

Config config;

/**
 * Bitwise stuff
 * index initial: 000 (em binário)
 * a.x < b.x:     001
 * a.y < b.y:     010
 * a.z < b.z:     100
 *
 * For each axis, one of the bits is set to 1 if a < b in that axis
 * Why? still dont know
 */
inline int getIndex(const Point& a, const Point& b) {
  int index = 0;
  if (a.x < b.x)
    index += 1;
  if (a.y < b.y)
    index += 2;
  if (a.z < b.z)
    index += 4;
  return index;
}

/**
 * Used to move a point by a given space in each axis
 * Index comes from the getIndex function up there.
 * 
 * index is used to know in which direction to update every axis
 */
inline void updateCenter(Point& p, int index, double radius) {
  for (int i = 0; i < 3; i++) {
    double v = (index & (1 << i)) > 0 ? radius : -radius;
    p[i] += v;
  }
}

typedef std::vector<Body>   Bodies;

typedef std::vector<Body*>  BodiesPtr;
typedef std::vector<BodiesPtr*> BodyBlocks;

#include "f_BuildOctree.h"
#include "f_ComputeCenterOfMass.h"
/** Build blocks of bodies */
#include "f_BodyBlocksBuild.h"
/** Compute Forces was replaced with CleanComputeForces, and later with BlockedComputeForces */
#include "f_CleanComputeForces.h"
#include "f_BlockedComputeForces.h"
#include "f_AdvanceBodies.h"
#include "f_ReduceBoxes.h"

double nextDouble() {
  return rand() / (double) RAND_MAX;
}

/**
 * Generates random input according to the Plummer model, which is more
 * realistic but perhaps not so much so according to astrophysicists
 */
void generateInput(Bodies& bodies, int nbodies, int seed) {
  double v, sq, scale;
  Point p;
  double PI = boost::math::constants::pi<double>();

  srand(seed);

  double rsc = (3 * PI) / 16;
  double vsc = sqrt(1.0 / rsc);

  int nextId = 0;

  for (int body = 0; body < nbodies; body++) {
    double r = 1.0 / sqrt(pow(nextDouble() * 0.999, -2.0 / 3.0) - 1);
    do {
      for (int i = 0; i < 3; i++)
        p[i] = nextDouble() * 2.0 - 1.0;
      sq = p.x * p.x + p.y * p.y + p.z * p.z;
    } while (sq > 1.0);
    scale = rsc * r / sqrt(sq);

    Body b;
    b.mass = 1.0 / nbodies;
    for (int i = 0; i < 3; i++)
      b.pos[i] = p[i] * scale;

    do {
      p.x = nextDouble();
      p.y = nextDouble() * 0.1;
    } while (p.y > p.x * p.x * pow(1 - p.x * p.x, 3.5));
    v = p.x * sqrt(2.0 / sqrt(1 + r * r));
    do {
      for (int i = 0; i < 3; i++)
        p[i] = nextDouble() * 2.0 - 1.0;
      sq = p.x * p.x + p.y * p.y + p.z * p.z;
    } while (sq > 1.0);
    scale = vsc * v / sqrt(sq);
    for (int i = 0; i < 3; i++)
      b.vel[i] = p[i] * scale;

    bodies.push_back(b);
    b.id = nextId;
    nextId++;
  }
}

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

void run(int nbodies, int ntimesteps, int seed) {
  Bodies bodies;
  BodyBlocks body_blocks;
  SpatialBodySortingTraits sst;

  generateInput(bodies, nbodies, seed);
  /*for(int i = 0; i < nbodies; ++i)
    std::cout << "body " << i << " " << bodies[i] << std::endl;*/

  typedef GaloisRuntime::WorkList::dChunkedLIFO<256> WL;

  //
  // Main loop
  //
  for (int step = 0; step < ntimesteps; step++) {


    // Do tree building sequentially
    Galois::setActiveThreads(1);

    //
    // Step 0.1. Body ordering goes here
    // 
    CGAL::spatial_sort(bodies.begin(), bodies.end(), sst);

    //
    // Step 0.2. BodyBlocks build
    //
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

    /*for(int i = 0; i < nbodies; ++i)
      std::cout << "body " << i << " " << &bodies[i] << std::endl;

    for(uint i = 0; i < body_blocks.size(); ++i)
      for(uint j = 0; j < body_blocks[j].size(); ++j)
        std::cout << "body " << i << " in block " << j << " " << &body_blocks[i][j] << std::endl;

      std::cout << "body 0 in node " << 0 << " " << top->child[0] << std::endl;
      std::cout << "body 1 in node " << 1 << " " << top->child[1] << std::endl;
*/

    // Parallel stuff starts here
    Galois::StatTimer T_parallel("ParallelTime");
    T_parallel.start();
    Galois::setActiveThreads(numThreads);

    //
    // Step 4. Compute forces for each body
    //
    Galois::for_each<WL>(wrap(bodies.begin()), wrap(bodies.end()),
        CleanComputeForces(top, box.diameter()));
    //Galois::for_each<WL>(wrap(body_blocks.begin()), wrap(body_blocks.end()),
    //      BlockedComputeForces(top, box.diameter()));

    //
    // Step 5. Update body positions
    //
    Galois::for_each<WL>(wrap(bodies.begin()), wrap(bodies.end()),
        AdvanceBodies());
    T_parallel.stop();

    std::cout 
      << "Timestep " << step
      << " Center of Mass = " << top->pos << "\n";
    std::cout
      << "Time measured: " << T_parallel.get_usec() << std::endl;
    delete top;
  }

  if (output_final) {
    std::cout << std::endl << "Final positions:" << std::endl;
    for(int i = 0; i < nbodies; ++i) {
      std::cout << i << ", " << bodies[i].pos << std::endl;
    }
  }
}

} // end namespace

int main(int argc, char** argv) {
  Galois::StatManager M;
  LonestarStart(argc, argv, name, desc, url);
  std::cout.setf(std::ios::right|std::ios::scientific|std::ios::showpoint);

  std::cerr << "configuration: "
            << nbodies << " bodies, "
            << ntimesteps << " time steps" << std::endl << std::endl;
  std::cout << "Num. of threads: " << numThreads << std::endl;

  Galois::StatTimer T;
  T.start();
  run(nbodies, ntimesteps, seed);
  T.stop();
}
