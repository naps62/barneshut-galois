#ifndef _BVHTREE_H
#define _BVHTREE_H

#include <ostream>
#include <vector>

#include "BVHNode.h"

class BVHTree {

	BVHNode *root;

	public:
		/**
		 * Constructors
		 */
		BVHTree(std::vector<Object*>& elems);

		// returns true if the Ray intersects an object of the tree, also giving the distance and object as secondary results
		bool intersect(const Ray& r, double& dist, Object *& obj) const;

		// dumps the entire tree in DOT format
		void dumpDot(std::ostream& os) const;

	private:
		// build the tree from a given collection of objects
		void buildTree(std::vector<Object*>& elems);
};

#endif // _BVHTREE_H