/**
 * BVHTree
 */
struct BVHTree : BoundingBox {
	static uint nextId;
	const uint id;
	BVHTree *parent;
	bool leaf;
	void *childs[2];

	BVHTree() : id(nextId++) {
		parent = NULL;
		childs[0] = NULL; childs[1] = NULL;
		
	}

	BVHTree(BVHTree* _parent)
	:	id(nextId++),
		parent(_parent)
		 {

		childs[0] = NULL; childs[1] = NULL;
	}

	void updateTreeBox(const BoundingBox& box) {
		this->containBox(box);
		if (parent)
			parent->updateTreeBox(box);
	}

	void dump(std::ostream& os);
};
uint BVHTree::nextId = 0;

std::ostream& operator<<(std::ostream& os, const BVHTree p) {

		os << "\t" << p.id << " [width=.5,height=1,style=filled,color=\".5 .5 .5\",shape=box,label=\"" << p.id << ",\\n" << p.min << ",\\n" << p.max << "\"];" << endl;
		if (p.leaf) {
			for(uint i = 0; i < 2; ++i) {
				Sphere* o = static_cast<Sphere*>(p.childs[i]);
				if (o)
					os << "\t" << p.id << " -> " << "b" << o->id << endl << *o;
			}
			os << endl;
		} else {
			for(uint i = 0; i < 2; ++i) {
				BVHTree& o = *static_cast<BVHTree*>(p.childs[i]);
				os << "\t" << p.id << " -> " << o.id << endl << o;
			}
			os << endl;
		}

	return os;
	/*os << "{" << p.min << ", " << p.max << ", ";
	for(uint i = 0; i <= 1; ++i) {
		if (p.childs[i]) {
			if (p.leaf) os << *(static_cast<Sphere*>(p.childs[i]));
			else			os << *static_cast<BVHTree*>(p.childs[i]);
		}
		os << "\t";
	}
	os << "}";

	return os;*/
}

void BVHTree::dump(std::ostream& os) {
	os << "digraph tree {" << endl
		<< *this << endl
		<< "}" << endl;
}