#include "octree.h"

/////////////////////////////////////
//  Structures
//
struct OctreeLeaf : Octree {
	OctreeObject* intersect (double, double, double, double tx1, double ty1, double tz1);
};

/////////////////////////////////////
//  Definitions - OctreeLeaf
//
inline
OctreeLeaf* OctreeLeaf::intersect (double, double, double, double tx1, double ty1, double tz1)
{
	if (tx1 < 0 || ty1 < 0 || tz1 < 0)
		return NULL;
	else
		return this;
}
