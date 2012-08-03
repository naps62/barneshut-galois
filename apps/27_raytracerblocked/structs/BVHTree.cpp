#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <sstream>

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

bool BVHTree::intersect (const RayList& rays, ColisionMap& colisions) const {
	return root->intersect(rays, colisions);
	// for (unsigned i = 0; i < rays.size(); ++i)
	// 	std::cout << *rays[i] << std::endl;
}

bool BVHTree::intersect (Ray** const rays, const unsigned nrays, ColisionMap& colisions) const {
	RayList v(rays, rays + nrays);
	return intersect(v, colisions);
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
