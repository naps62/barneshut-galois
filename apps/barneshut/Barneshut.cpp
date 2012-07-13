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
#include <strings.h>
#include <boost/math/constants/constants.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include "Galois/Galois.h"
#include "Galois/Statistic.h"
#include "llvm/Support/CommandLine.h"
#include "Lonestar/BoilerPlate.h"

namespace {
const char* name = "Barnshut N-Body Simulator";
const char* desc =
  "Simulation of the gravitational forces in a galactic cluster using the "
  "Barnes-Hut n-body algorithm\n";
const char* url = "barneshut";

static llvm::cl::opt<int> nbodies("n", llvm::cl::desc("Number of bodies"), llvm::cl::init(10000));
static llvm::cl::opt<int> ntimesteps("steps", llvm::cl::desc("Number of steps"), llvm::cl::init(1));
static llvm::cl::opt<int> seed("seed", llvm::cl::desc("Random seed"), llvm::cl::init(7));

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

typedef std::vector<Body> Bodies;

#include "f_BuildOctree.h"
#include "f_ComputeCenterOfMass.h"
#include "f_ComputeForces.h"
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
  }
}

template<typename T>
struct Deref : public std::unary_function<T, T*> {
  T* operator()(T& item) const { return &item; }
};

boost::transform_iterator<Deref<Body>, Bodies::iterator> 
wrap(Bodies::iterator it) {
  return boost::make_transform_iterator(it, Deref<Body>());
}

void run(int nbodies, int ntimesteps, int seed) {
  Bodies bodies;
  generateInput(bodies, nbodies, seed);

  typedef GaloisRuntime::WorkList::dChunkedLIFO<256> WL;

  //
  // Main loop
  //
  for (int step = 0; step < ntimesteps; step++) {

    //
    // Step 1. Generate a bounding box that contains all points. This is done sequentially
    //

    // Do tree building sequentially
    Galois::setActiveThreads(1);

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


    //
    // Parallel stuff starts here
    //
    Galois::StatTimer T_parallel("ParallelTime");
    T_parallel.start();
    Galois::setActiveThreads(numThreads);

    //
    // Step 4. Compute forces for each body
    //
    Galois::for_each<WL>(wrap(bodies.begin()), wrap(bodies.end()),
        ComputeForces(top, box.diameter()));

    //
    // Step 5. Update body positions
    //
    Galois::for_each<WL>(wrap(bodies.begin()), wrap(bodies.end()),
        AdvanceBodies());
    T_parallel.stop();

    std::cout 
      << "Timestep " << step
      << " Center of Mass = " << top->pos << "\n";
    delete top;
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
