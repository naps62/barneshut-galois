/**
 * BVHTree
 */
struct BVHTree : BoundingBox {
	BVHTree *parent;
	bool leaf;
	void *childs[2];

	BVHTree() {
		parent = NULL;
		childs[0] = NULL; childs[1] = NULL;
	}

	BVHTree(BVHTree* _parent)
	:	parent(_parent) {

		childs[0] = NULL; childs[1] = NULL;
	}

	void updateTreeBox(const BoundingBox& box) {
		this->containBox(box);
		if (parent)
			parent->updateTreeBox(box);
	}
};

std::ostream& operator<<(std::ostream& os, const BVHTree p) {
	os << "{" << p.min << ", " << p.max << ", ";
	for(uint i = 0; i <= 1; ++i) {
		if (p.childs[i]) {
			if (p.leaf) os << *(static_cast<Sphere*>(p.childs[i]));
			else			os << *static_cast<BVHTree*>(p.childs[i]);
		}
		os << "\t";
	}
	os << "}";

	return os;
}