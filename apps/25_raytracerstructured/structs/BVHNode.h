#ifndef _BVHNODE_H
#define _BVHNODE_H

#include <ostream>

#include "Box.h"

/**
 * BVHTree
 */
struct BVHNode {
	// static counter for unique ID's
	static uint nextId;

	// instance ID
	const uint id;

	// parent node
	BVHNode *parent;

	// bouding box describing this node
	Box box;

	// is this a leaf node?
	bool leaf;

	// childs (BVHNode* if !leaf, Object* otherwise)
	void *childs[2];

	//
	// Methods
	//

	/**
	 * Constructors
	 */
	BVHNode();
	BVHNode(BVHNode* _parent);

	// updates bounding box on this node, and recurses up the tree
	void updateTreeBox(const Box& box);

	/**
	 * Output
	 */
	//friend std::ostream& operator<<(std::ostream& os, const BVHNode& p);
	std::string toString() const;
	void dumpDot(std::ostream& os) const;
};

#endif // _BVHNODE_H