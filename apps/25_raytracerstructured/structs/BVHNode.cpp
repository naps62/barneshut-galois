#include <string>
#include <sstream>

#include "BVHNode.h"

uint BVHNode::nextId = 0;

/**
 * Constructors
 */
BVHNode::BVHNode()
: id(nextId++)
{
	parent = NULL;
	childs[0] = NULL; childs[1] = NULL;		
}


BVHNode::BVHNode(BVHNode* _parent)
:	id(nextId++),
	parent(_parent)
{
	childs[0] = NULL; childs[1] = NULL;
}


/**
 * Methods
 */
void BVHNode::updateTreeBox(const Box& b) {
	this->box.containBox(b);
	if (parent)
		parent->updateTreeBox(b);
}


/**
 * Output
 */
std::string BVHNode::toString() const {
	using std::stringstream;
	using std::endl;

	stringstream ss;
	ss << "\t" << id << " [width=.5,height=1,style=filled,color=\".5 .5 .5\",shape=box,label=\"" << id << ",\\n" << box.min << ",\\n" << box.max << "\"];" << std::endl;
	if (leaf) {
		for(uint i = 0; i < 2; ++i) {
			Object* o = static_cast<Object*>(childs[i]);
			if (o) ss << "\t" << id << " -> " << "box" << o->id << std::endl << o->toString();
		}
		ss << std::endl;
	} else {
		for(uint i = 0; i < 2; ++i) {
			BVHNode& o = *static_cast<BVHNode*>(childs[i]);
			ss << "\t" << id << " -> " << o.id << std::endl << o.toString();
		}
		ss << std::endl;
	}
	return ss.str();
}

void BVHNode::dumpDot(std::ostream& ss) const {
	ss << "digraph tree {" << std::endl
		<< this->toString() << std::endl
		<< "}" << std::endl;
}