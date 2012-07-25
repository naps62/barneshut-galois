// smallpt, a Path Tracer by Kevin Beason, 2008 
// Make : g++ -O3 -fopenmp smallpt.cpp -o smallpt 
//        Remove "-fopenmp" for g++ version < 4.2 

#include <cmath>
#include <algorithm>
#include <limits>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <set>
using namespace std;

#include "Galois/Galois.h"
#include "Galois/Statistic.h"
#include <Galois/Runtime/ll/SimpleLock.h>
#include "Lonestar/BoilerPlate.h"


#include "structs/All.h"

/** clamps a value between 0 and 1 */
inline double clamp(double x) {
	if (x < 0)      return 0;
	else if (x > 1) return 1;
	else            return x;
}

/** interpolates a double to a 0-255 integer scale */
inline int toInt(double x) {
	return int(pow(clamp(x), 1 / 2.2) * 255 + 0.5);
} 


typedef vector<Ray> Rays;
typedef vector<Object*> ObjectList;

#include "f_RayTrace.h"
#include "f_ClampImage.h"
#include "scene.h"

const char* name = "RayTracing";
const char* desc = "Simple ray tracing implementation using octree for scene indexing\n";
const char* url  = "raytracing";

static llvm::cl::opt<uint>   width   ("w",    llvm::cl::desc("Output image width"),  llvm::cl::init( 1024));
static llvm::cl::opt<uint>   height  ("h",    llvm::cl::desc("Output image height"), llvm::cl::init(  768));
static llvm::cl::opt<uint>   spp     ("spp",  llvm::cl::desc("Samples per pixel"),   llvm::cl::init(    4));
static llvm::cl::opt<uint>   maxdepth("d",    llvm::cl::desc("Max ray depth"),       llvm::cl::init(    5));
static llvm::cl::opt<int>    seed    ("seed", llvm::cl::desc("Random seed"),         llvm::cl::init(    7));
static llvm::cl::opt<uint>   n       ("n",    llvm::cl::desc("Number of spheres"),   llvm::cl::init(    2));
static llvm::cl::opt<uint>   dump    ("dump", llvm::cl::desc("Dump BVH Tree"),       llvm::cl::init(false));
static llvm::cl::opt<string> outfile ("out",  llvm::cl::desc("Output file"),         llvm::cl::init(string("image.ppm")));



/***********************************************************
 * MAIN
 **********************************************************/
// Usage: time ./smallpt 5000 && xv image.ppm
int main(int argc, char *argv[]) {
	Galois::StatManager M;
	LonestarStart(argc, argv, name, desc, url);

	cerr << fixed;
	cerr.width(2);
	cerr.precision(2);
	cerr << endl;

	std::cerr	<< "configuration: "
					<< width << "x" << height
					<< " (" << spp << " spp)"
					<< std::endl << std::endl;

	std::cout << "Num. of threads: " << numThreads << std::endl;
	

	Galois::StatTimer T("Total");
	T.start();
	Scene scene(width, height, spp, maxdepth, n, dump);
	scene.raytrace();
	scene.save(outfile);
	T.stop();

	cerr << endl << endl;
}