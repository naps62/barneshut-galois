#include <string>
#include <sstream>
#include <algorithm>

#include "BVHTree.h"

/**
 * Constructors
 */
BVHTree::BVHTree(std::vector<Object*>& elems) {
	this->buildTree(elems);
}

/**
 * Methods
 */
bool BVHTree::intersect(const Ray& r, double& dist, Object *& obj) const {
	return root->recurseTree(r, dist, obj);
}

bool intersectBlock(vector<Ray*> rays, std::vector<std::pair<double, Object*>& colisions) const {
	return root->blockedRecurseTree(rays, colisions);
}


/**
 * Tree creation
 */
void BVHTree::buildTree(std::vector<Object*>& elems) {
	std::vector<Object*> tmp_elems;

	this->root = new BVHNode(NULL, elems, tmp_elems, 0, elems.size() - 1, 0);

	for(uint i = 0; i < elems.size(); ++i) {
		elems[i] = tmp_elems[i];
	}
}


void BVHTree::dumpDot(std::ostream& ss) const {
	ss << "digraph tree {" << std::endl
		<< root->toString() << std::endl
		<< "}" << std::endl;
}