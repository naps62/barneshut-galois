#ifndef ___OCTREE___
#define ___OCTREE___

#include "point.h"
#include "ray.h"

struct OctreeLeaf;

struct Octree {
	Point pos;

	// virtual OctreeLeaf* intersect (double, double, double, double, double, double) = 0;

	virtual bool isLeaf() const = 0;

	/** Computes where the ray intersects the octree.
	 * \param r Ray to intersect.
	 * \param id Reference to an integer, used to return the id of the first intersected object.
	 * \return The squared distance between the ray origin and the intersected node/object. The squared distance is more efficient and holds equally for comparisons.
	 */
	virtual OctreeLeaf* intersect (const Ray &r) = 0;
protected:
	Octree () : pos(0.0) { }
	Octree (Point _pos) : pos(_pos) { }
};

#include "octreeinternal.h"
#include "octreeleaf.h"

#endif//___OCTREE___
