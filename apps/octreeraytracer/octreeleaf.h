#ifndef ___OCTREE_LEAF___
#define ___OCTREE_LEAF___

#include "octree.h"

/////////////////////////////////////
//  Structures
//
struct OctreeLeaf : public Octree {
	int id;

	//	Constructors
	OctreeLeaf (Point pos) : Octree(pos) {}
	OctreeLeaf (Point pos, int _id) : Octree(pos), id(_id) {}

	// OctreeLeaf* intersect (double, double, double, double tx1, double ty1, double tz1);

	bool isLeaf() const { return true; }
	virtual OctreeLeaf* intersect (const Ray &r) = 0;
};

/////////////////////////////////////
//  Definitions - OctreeLeaf
//
// inline
// OctreeLeaf* OctreeLeaf::intersect (double, double, double, double tx1, double ty1, double tz1)
// {
// 	if (tx1 < 0 || ty1 < 0 || tz1 < 0)
// 		return NULL;
// 	else
// 		return this;
// }

//  Output operator for the leaf octree nodes.
inline
std::ostream& operator<<(std::ostream& os, const OctreeLeaf& b) {
	os << "(pos:" << b.pos << ")";
	return os;
}

#endif//___OCTREE_LEAF___
