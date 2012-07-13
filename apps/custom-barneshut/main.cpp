
#include <iostream>
#include <strings.h>

#include "Galois/Galois.h"
#include "Lonestar/BoilerPlate.h"
#include "Barneshut.h"

int main(int argc, char** argv) {
  LonestarStart(argc, argv, std::cout, name, desc, url);
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