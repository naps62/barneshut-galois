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
#include "point.h"
#include "kdtree.h"
#include "utilities.h"
	





// template<unsigned K>
// unsigned correlated (vector<Point<K>*>& points, const double radius, KdTree<K>& tree) {
// 	unsigned count = 0;
// 	for (unsigned i = 0; i < points.size(); ++i)
// 		count += tree.correlated(*points[i], radius);
// 	return (count - points.size()) / 2;
// }





const char* name = "Two-Point Correlation";
const char* desc = "Counting the pairs of points which lie in a given radius.";
const char* url = "pointcorrelation";

static llvm::cl::opt<unsigned> npoints("n", llvm::cl::desc("Number of points"), llvm::cl::init(32));
static llvm::cl::opt<double> radius("r", llvm::cl::desc("Threshold radius"), llvm::cl::init(3));
static llvm::cl::opt<unsigned> threads("threads", llvm::cl::desc("Number of threads to use"), llvm::cl::init(1));
static llvm::cl::opt<unsigned> seed("seed", llvm::cl::desc("Pseudo-random number generator seed. Defaults to current timestamp"), llvm::cl::init(time(NULL)));

#define DIM 3

struct PointSpatialSortingTraits {
	typedef Point<3>* Point_3;

	typedef struct {
		bool operator() (const Point<3>* const p, const Point<3>* const q) const { return (*p)[0] < (*q)[0]; }
	} Less_x_3;

	typedef struct {
		bool operator() (const Point<3>* const p, const Point<3>* const q) const { return (*p)[1] < (*q)[1]; }
	} Less_y_3;

	typedef struct {
		bool operator() (const Point<3>* const p, const Point<3>* const q) const { return (*p)[2] < (*q)[2]; }
	} Less_z_3;

	Less_x_3 less_x_3_object() const { return Less_x_3(); }
	Less_y_3 less_y_3_object() const { return Less_y_3(); }
	Less_z_3 less_z_3_object() const { return Less_z_3(); }
};


int main (int argc, char *argv[]) {
	LonestarStart(argc, argv, name, desc, url);

	vector<Point<DIM>*> points;

	generateInput(points, npoints, seed);

	//	Sort points
	// PointSpatialSortingTraits sst;
	CGAL::spatial_sort(points.begin(), points.end(), PointSpatialSortingTraits());

	KdTree<DIM> tree(points);

	// two point correlation
	KdTree<DIM>::Correlator correlator(tree, radius);
	// Galois::setActiveThreads(1);
	Galois::setActiveThreads(threads);
	Galois::StatTimer t;
	count.reset(0);
	t.start();
	// unsigned count = tree.correlated(points, radius, std::cerr);
	Galois::for_each(Point<DIM>::wrap(points.begin()), Point<DIM>::wrap(points.end()), correlator);
	t.stop();
	std::cerr << "\t\t" << (double) t.get_usec() * 1e-6 << " seconds" << std::endl;
	// std::cout << count << std::endl;
	std::cout << (count.get() - points.size()) / 2 << std::endl;

	//	CLEANUP
	for (unsigned i = 0; i < points.size(); ++i)
		delete points[i];

	return 0;
}
