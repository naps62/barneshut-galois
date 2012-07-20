#include "point.h"

struct OctreeLeaf;

struct Octree {
	Point _pos;


	
	virtual OctreeLeaf* intersect (double, double, double, double, double, double) = 0;
protected:
	Octree (Point pos) : _pos(pos) { }
};


