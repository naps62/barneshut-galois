#include <string>
#include <sstream>
#include <algorithm>

#include "BVHNode.h"
#include "Sphere.h"
#include "UID.h"

/**
 * Constructors
 */
BVHNode::BVHNode()
:	id(UID::get()),
	parent(NULL)
{
	childs[0].node = NULL; childs[1].node = NULL;
}


BVHNode::BVHNode(BVHNode * const _parent)
:	id(UID::get()),
	parent(_parent)
{
	childs[0].leaf = NULL; childs[1].leaf = NULL;
}

BVHNode::BVHNode(	BVHNode * const _parent,
						std::vector<Object*>& elems,
						std::vector<Object*>& tmp_elems,
						uint start,
						uint end,
						uint axis)
:	id(UID::get()),
	parent(_parent)
{
	buildNode(elems, tmp_elems, start, end, axis);
}


/**
 * Methods
 */
void BVHNode::updateTreeBox(const Box& b) {
	this->box.containBox(b);
	if (parent)
		parent->updateTreeBox(b);
}


bool BVHNode::recurseTree(const Ray& ray, double& dist, Object *& obj) {
	const double inf = 1e20;
	double d;
	dist = inf;

	if (! this->box.isIntersected(ray)) {
		return false;
	}

	Object* current_obj;

	if (this->leaf) {
		for (uint i = 0; i < 2; ++i) {
			current_obj = this->childs[i].leaf;
			if (current_obj && (d = current_obj->intersect(ray)) && d < dist) {
				dist = d;
				obj  = current_obj;
			}
		}
	} else {
		double d;
		int inner_id;
		// recurse to left branch
		if (this->childs[0].node->recurseTree(ray, d, current_obj) && d < dist) {
			dist = d;
			obj  = current_obj;
		}
		// recurse to right branch
		if (this->childs[1].node->recurseTree(ray, d, current_obj) && d < dist) {
			dist = d;
			obj  = current_obj;
		}
	}
	return dist < inf;
}

struct Frame {
	
}

bool intersectBlock(vector<Ray*> rays, std::vector<std::pair<double, Object*>& colisions) const {
	const double inf = 1e20;

	if (this->leaf) {
		for(uint i = 0; i < rays.size(); ++i) {
			const double dist = colisions[i].first;
			Object* obj_ptr = colisions[i].second;
			Object* current_obj = this->childs[i].leaf;
			Ray& ray = *(rays[i]);

			if (obj_ptr && current_obj && (d = current_obj->intersect(ray)) && d < dist) {
				colisions[i].first = 
			}
		}
	} else {

	}
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
			Object* o = childs[i].leaf;
			if (o) ss << "\t" << id << " -> " << "box" << o->id << std::endl << o->toString();
		}
		ss << std::endl;
	} else {
		for(uint i = 0; i < 2; ++i) {
			BVHNode* o = childs[i].node;
			ss << "\t" << id << " -> " << o->id << std::endl << o->toString();
		}
		ss << std::endl;
	}
	return ss.str();
}


void BVHNode::sortObjects(	std::vector<Object*>& elems,
									const uint start,
									const uint end,
									const uint axis) {

	// insert into an aux vector
	std::vector<Object*> sorted;
	for(uint i = start; i <= end; ++i)
		sorted.push_back(elems[i]);

	// sort it
	sort(sorted.begin(), sorted.end(), ObjectComp_Axis(axis));

	// replace chunck of the original vector
	uint i = start;
	for(std::vector<Object*>::iterator it = sorted.begin(); it != sorted.end(); ++it, ++i)
		elems[i] = *it;

}

void BVHNode::buildNode(std::vector<Object*>& elems,
								std::vector<Object*>& tmp_elems,
								const uint start,
								const uint end,
								uint axis) {

	int count = end - start + 1;

	if (count <= 2) {
		Object *o1 = new Sphere(*static_cast<Sphere*>(elems[start]));
		// TODO remove this
		delete elems[start];
		tmp_elems.push_back(o1);

		this->leaf = true;
		this->childs[0].leaf = o1;

		// update node bounding box
		this->updateTreeBox(o1->box());

		if (count == 2) {
			// TODO remover isto depois do estagio. nao ha necessidade de reinstanciar as esferas
			Object *o2 = new Sphere(*static_cast<Sphere*>(elems[start + 1]));
			// TODO remove this
			delete elems[start+1];
			tmp_elems.push_back(o2);

			this->childs[1].leaf = o2;
			this->updateTreeBox(o2->box());
		} else {
			this->childs[1].leaf = NULL;
		}
	} else {
		sortObjects(elems, start, end, axis);

		uint center = start + (uint) (count * 0.5f);
		axis = (axis == 2) ? 0 : axis+1;

		this->leaf = false;
		this->childs[0].node = new BVHNode(this, elems, tmp_elems, start, center-1, axis);
		this->childs[1].node = new BVHNode(this, elems, tmp_elems, center, end, axis);
	}
}