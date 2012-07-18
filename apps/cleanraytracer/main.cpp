// smallpt, a Path Tracer by Kevin Beason, 2008 
// Make : g++ -O3 -fopenmp smallpt.cpp -o smallpt 
//        Remove "-fopenmp" for g++ version < 4.2 

#include <cmath>   
#include <stdlib.h> 
#include <assert.h>
#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;

#include "llvm/Support/CommandLine.h"

#include "vec.h"
#include "ray.h"
#include "object.h"
#include "scene.h"


static llvm::cl::opt<unsigned> width ("w",   llvm::cl::desc("Output image width"),  llvm::cl::init(1024));
static llvm::cl::opt<unsigned> height("h",   llvm::cl::desc("Output image height"), llvm::cl::init(768));
static llvm::cl::opt<unsigned> spp   ("spp", llvm::cl::desc("Samples per pixel"),   llvm::cl::init(1));

/***********************************************************
 * MAIN
 **********************************************************/
// Usage: time ./smallpt 5000 && xv image.ppm
int main(int argc, char *argv[]) {
	llvm::cl::ParseCommandLineOptions(argc, argv);
	cout << spp << endl;
	assert((spp % 4 == 0) && "Samples-per-pixel must be a multiple of 4");

	// cam pos, lookAt 
	Ray cam(Vec(50, 52, 295.6), Vec(0, -0.042612, -1).norm());

	Scene scene(width, height, Vec()/*TODO falta isto*/);
	scene.raytrace(spp);
	scene.save(string("image.ppm"));
}