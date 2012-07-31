#ifndef _BVHTREE_H
#define _BVHTREE_H

#include <ostream>
#include <vector>

#include "BVHNode.h"

struct BVHTree {

	BVHNode *root;

	/**
	 * Constructors
	 */
	BVHTree(std::vector<Object*>& elems);

	// returns true if the Ray intersects an object of the tree, also giving the distance and object as secondary results
	bool intersect(const Ray& r, double& dist, Object *& obj) const;

	bool intersect (const std::vector<Ray*>& rays, std::map<Ray*,std::pair<double, Object*> >& colisions) const;

	bool intersect (Ray** const rays, const unsigned nrays, std::map<Ray*,std::pair<double, Object*> >& colisions) const;

	// dumps the entire tree in DOT format
	void dumpDot(std::ostream& os) const;

	private:
	// build the tree from a given collection of objects
	void buildTree(std::vector<Object*>& elems);
};

#endif // _BVHTREE_H