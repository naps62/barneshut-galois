// C++ includes
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
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
static llvm::cl::opt<unsigned> threads("threads", llvm::cl::desc("Number of threads to use"), llvm::cl::init(1));
static llvm::cl::opt<unsigned> seed("seed", llvm::cl::desc("Pseudo-random number generator seed. Defaults to current timestamp"), llvm::cl::init(time(NULL)));
static llvm::cl::opt<unsigned> blocksize("bs", llvm::cl::desc("Block size (number of points to use in a block). Low values mean excessive number of instructions. High values exceed cache capacity."), llvm::cl::init(1));

#define DIM 3

int main (int argc, char *argv[]) {
	LonestarStart(argc, argv, name, desc, url);

	Point<DIM>::Block points;

	generateInput(points, npoints, seed);

	//	Sort points
	CGAL::spatial_sort(points.begin(), points.end(), PointSpatialSortingTraits());

	KdTree<DIM> tree(points);

	//
	unsigned result;//<	Final result.
	Galois::StatTimer t;
	const bool g = threads > 0;//	activate Galois
	const bool b = blocksize > 0;//	activate blocks

	typedef Point<DIM>::Block Block;
	vector<Block> blocks;

	//	Split into blocks
	if (b) {
		blocks = Point<DIM>::blocks(points, blocksize);
			// // debug
			// for (unsigned i = 0; i < blocks.size(); ++i) {
			// 	// std::cerr << blocks[i].size() << std::endl;
			// 	for (unsigned j = 0; j < blocks[i].size(); ++j)
			// 		std::cerr << *blocks[i][j] << '\t';
			// 	std::cerr << std::endl;
			// }
			// // return 0;
	}

	//	Prepare Galois
	if (g) {
		Galois::setActiveThreads(threads);
		count.reset(0);
	}

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
	
	
	// Galois::StatTimer t;
	// count.reset(0);
	// t.start();
	// unsigned count = 0;
	// for (unsigned i = 0; i < blocks.size(); ++i)
		// count += tree.correlated(blocks[i], radius);
	// Galois::for_each(Point<DIM>::wrap(blocks.begin()), Point<DIM>::wrap(blocks.end()), correlator);
	// Galois::for_each(Point<DIM>::wrap(points.begin()), Point<DIM>::wrap(points.end()), correlator);
	// t.stop();
	// std::cerr << "\t\t" << (double) t.get_usec() * 1e-6 << " seconds" << std::endl;
	// std::cout << (count - points.size()) / 2 << std::endl;
	std::cout << result << std::endl;

	//	CLEANUP
	for (unsigned i = 0; i < points.size(); ++i)
		delete points[i];

	return 0;
}
