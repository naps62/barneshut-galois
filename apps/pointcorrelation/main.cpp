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
static llvm::cl::opt<unsigned> blocksize("bs", llvm::cl::desc("Block size (number of points to use in a block). Low values mean excessive number of instructions. High values exceed cache capacity."), llvm::cl::init(1));
static llvm::cl::opt<bool> togglesort("sort", llvm::cl::desc("Toggle spatial sort."), llvm::cl::init(false));
static llvm::cl::opt<bool> g("g", llvm::cl::desc("Toggle Galois."), llvm::cl::init(false));
static llvm::cl::opt<string> papicn("papi", llvm::cl::desc("PAPI counter name."), llvm::cl::init(string("")));

#define DIM 3

int main (int argc, char *argv[]) {
	LonestarStart(argc, argv, name, desc, url);

	Point<DIM>::Block points;

	generateInput(points, npoints, seed);

	//	Sort points
	if (togglesort)
		CGAL::spatial_sort(points.begin(), points.end(), PointSpatialSortingTraits());

	KdTree<DIM> tree(points);

	//
	unsigned result;//<	Final result.
	Galois::StatTimer t;
	if (numThreads > 1)
		g = true;
	// const bool g = numThreads > 0;//	activate Galois
	const bool b = blocksize > 0;//	activate blocks

	typedef Point<DIM>::Block Block;
	vector<Block> blocks;

	//	Prepare PAPI
	int papiEventSet = PAPI_NULL;
	if (!papicn.empty()) {
		assert(PAPI_library_init(PAPI_VER_CURRENT) == PAPI_VER_CURRENT);
		assert(PAPI_thread_init(getTID) == PAPI_OK);
		assert(PAPI_create_eventset(&papiEventSet) == PAPI_OK);
		int event;
		char * name = strdup(papicn.c_str());
		assert(PAPI_event_name_to_code(name, &event) == PAPI_OK);
		free(name);
		assert(PAPI_add_event(papiEventSet, event) == PAPI_OK);
	}

	//	Split into blocks
	if (b)
		blocks = Point<DIM>::blocks(points, blocksize);

	//	Prepare Galois
	if (g)
		count.reset(0);

	//	two point correlation
	if (g && b) {
		KdTree<DIM>::BlockedCorrelator correlator(tree, radius);
		t.start();
		Galois::for_each(Point<DIM>::wrap(blocks.begin()), Point<DIM>::wrap(blocks.end()), correlator);
		t.stop();
		result = (count.get() - points.size()) / 2;
	} else if (g) {
		KdTree<DIM>::Correlator correlator(tree, radius);
		t.start();
		Galois::for_each(Point<DIM>::wrap(points.begin()), Point<DIM>::wrap(points.end()), correlator);
		t.stop();
		result = (count.get() - points.size()) / 2;
	} else if (b) {
		result = 0;
		t.start();
		for (unsigned i = 0; i < blocks.size(); ++i)
			result += tree.correlated(blocks[i], radius);
		t.stop();
		result = (result - points.size()) / 2;
	} else {
		result = 0;
		t.start();
		for (unsigned i = 0; i < points.size(); ++i)
			result += tree.correlated(*points[i], radius);
		t.stop();
		result = (result - points.size()) / 2;
	}
	std::cerr << "\t\t" << (double) t.get_usec() * 1e-6 << " seconds" << std::endl;
	std::cout << result << std::endl;

	//	Cleanup PAPI
	if (papiEventSet != PAPI_NULL) {
		PAPI_cleanup_eventset(papiEventSet);
		PAPI_destroy_eventset(&papiEventSet);
		PAPI_shutdown();
	}

	//	CLEANUP
	for (unsigned i = 0; i < points.size(); ++i)
		delete points[i];

	return 0;
}
