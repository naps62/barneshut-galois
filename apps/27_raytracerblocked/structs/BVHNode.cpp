#include <algorithm>
#include <limits>
#include <map>
#include <string>
#include <sstream>
#include <utility>

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


bool BVHNode::recurseTree(const Ray& ray, double& dist, Object *& obj) const {
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
		// int inner_id;
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

bool BVHNode::intersect (const std::vector<Ray*>& rays, std::map<Ray*,std::pair<double, Object*> >& colisions) const {
	std::vector<Ray*> subrays;

	//	filter out all the rays which do not intersect the big box
	for (unsigned i = 0; i < rays.size(); ++i)
		if (box.isIntersected(*rays[i]))
			subrays.push_back(rays[i]);

	//	if no ray is alive, fail now
	if (subrays.size() == 0)
		return false;

	bool result = false;
	if (leaf) {
		//	if this node is a leaf, test each ray in each child
		for (unsigned i = 0; i < subrays.size(); ++i) {
			double distance = std::numeric_limits<double>::max();
			Object* object = NULL;
			for (unsigned j = 0; j < 2; ++j) {
				Object* o = childs[i].leaf;
				double d;
				if (o && (d = o->intersect(*subrays[i])) < distance) {
					distance = d;
					object = o;
				}
			}
			if (distance < std::numeric_limits<double>::max()) {
				colisions[subrays[i]] = std::make_pair(distance, object);
				result = true;
			}
		}
	} else {
		for (unsigned i = 0; i < 2; ++i) {
			std::map<Ray*,std::pair<double,Object*> > subcolisions;
			//	recursively find subsequent ray colisions
			result = result || childs[i].node->intersect(subrays, subcolisions);

			//	merge colisions
			std::map<Ray*,std::pair<double,Object*> >::iterator it;
			for (it = subcolisions.begin(); it != subcolisions.end(); ++it)
				if (colisions.count(it->first) > 0) {
					double d1 = colisions[it->first].first;
					double d2 = it->second.first;
					if (d2 < d1)
						colisions[it->first] = it->second;
				} else
					colisions[it->first] = it->second;
		}
	}
	return result;
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
