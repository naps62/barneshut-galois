#ifndef ___OCTREE_INTERNAL___
#define ___OCTREE_INTERNAL___

#include "octree.h"
#include "octreeleaf.h"

// C++ includes
#include <iostream>

//	C includes
#include <string.h>

#include "boundingbox.h"
#include "ray.h"

/**	Non-leaf octree node. Represents a voxel as an object container.
 */
class OctreeInternal : public Octree {
	Point _min, _max;
	int _node;
public:
	Octree* childs[8];
	const Point size;//	size of this voxel

	/**
	 * \param pos Centroid (current node) in cartesian coordinates.
	 */
	OctreeInternal (Point pos, Point dimension)
	: Octree(pos)
	, size(dimension)
	{ init(); }

	OctreeInternal (BoundingBox bbox)
	: Octree(bbox.center())
	, _min(bbox.min)
	, _max(bbox.max)
	, size(abs(_max.x - _min.x), abs(_max.y - _min.y), abs(_max.z - _min.z))
	{
		init();
	}

	void insert(OctreeLeaf *obj);

	// OctreeLeaf* shoot (Ray x);

	bool isLeaf() const { return false; }

	//	returns the first intersected object in this node
	// OctreeLeaf* intersect (double tx0, double ty0, double tz0, double tx1, double ty1, double tz1);
	OctreeLeaf* intersect (const Ray &r);
private:

	void init() {
		memset(childs, NULL, sizeof(*childs) * 8);
	}

	/**	Returns the index of the sub-voxel a leaf belongs in.
	 * \param p Position of the leaf.
	 */
	int index(const Point& p);

	double max(double x, double y, double z);
	double min(double x, double y, double z);

	//	returns the index of the first sub-node intersected by the ray
	int first_node (double tx0, double ty0, double tz0, double txm, double tym, double tzm);

	//	returns the index of the next sub-node to be intersected by the ray
	int new_node (double f1, int i1, double f2, int i2, double f3, int i3);

	// void updateCenter(Point& p, int index, double radius);

	//	returns the centroid of a sub-voxel identified by its index
	Point centroid(int index);
};



/////////////////////////////////////
//  Definitions - OctreeInternal
//
inline
double max(double x, double y, double z)
{ return (x > y) ? ((x > z) ? x : z) : ((y > z ? y : z)); }

inline
double min(double x, double y, double z)
{ return (x < y) ? ((x < z) ? x : z) : ((y < z ? y : z)); }

inline
int OctreeInternal::index(const Point& p)
{
	return
		(p.x > pos.x) * 4 +
		(p.y > pos.y) * 2 +
		(p.z > pos.z) * 1;
}

inline
int new_node (double f1, int i1, double f2, int i2, double f3, int i3)
{
	if (f1 < f2) {
		if (f1 < f3)
			return i1;
	} else {
		if (f2 < f3)
			return i2;
	}
	return i3;
}

inline
Point OctreeInternal::centroid (int index)
{
	Point p(pos);
	Point sizehf(size * .5);
	//	move in the x axis
	if (index & 4)
		p.x += sizehf.x;
	else
		p.x -= sizehf.x;
	//	move in the y axis
	if (index & 2)
		p.y += sizehf.y;
	else
		p.y -= sizehf.y;
	//	move in the z axis
	if (index & 1)
		p.z += sizehf.z;
	else
		p.z -= sizehf.z;
	return p;
}



// inline
// void updateCenter(Point& p, int index, double radius)
// {
//   for (int i = 0; i < 3; i++) {
//     double v = (index & (1 << i)) > 0 ? radius : -radius;
//     p[i] += v;
//   }
// }

inline
std::ostream& operator<< (std::ostream& out, OctreeInternal& octint) {
	out << octint.pos << '\t' << octint.size;
	return out;
}

#endif//___OCTREE_INTERNAL___
