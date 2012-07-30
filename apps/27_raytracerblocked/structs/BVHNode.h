#ifndef _BVHNODE_H
#define _BVHNODE_H

#include <vector>

#include "BVHNode.h"
#include "Box.h"

/**
 * BVHTree
 */
struct BVHNode {
	// instance ID
	const uint id;

	// parent node
	BVHNode * const parent;

	// bouding box describing this node
	Box box;

	// is this a leaf node?
	bool leaf;

	// childs (BVHNode* if !leaf, T* otherwise)
	union {
		BVHNode *node;
		Object *leaf;
	} childs[2];

	//
	// Methods
	//

	/**
	 * Constructors
	 */
	BVHNode();

	BVHNode(BVHNode * const _parent);

	BVHNode(	BVHNode * const _parent,
				std::vector<Object*>& elems,
				std::vector<Object*>& tmp_elems,
				const uint start,
				const uint end,
				const uint axis);

	// updates bounding box on this node, and recurses up the tree
	void updateTreeBox(const Box& box);

	bool recurseTree(const Ray& r, double& dist, Object *& obj);

	/**
	 * Output
	 */
	//friend std::ostream& operator<<(std::ostream& os, const BVHNode& p);
	std::string toString() const;

	private:
	void sortObjects(	std::vector<Object*>& elems,
							const uint start,
							const uint end,
							const uint axis);

	void buildNode(std::vector<Object*>& elems,
						std::vector<Object*>& tmp_elems,
						uint start,
						uint end,
						uint axis);
};

#endif // _BVHNODE_H