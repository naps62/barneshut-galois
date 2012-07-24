#include <limits>
#include <sstream>

#include "Box.h"

/**
 * Constructors
 */
Box::Box() 
:	min(true,  std::numeric_limits<double>::max()),
	max(true, -std::numeric_limits<double>::max())
{ }


Box::Box(Vec _min, Vec _max)
:	min(_min),
	max(_max)
{ }


void Box::containBox(const Box& box) {
	min = min.min(box.min);
	max = max.max(box.max);
}
	

/**
 * Methods
 */
bool Box::isIntersected (const Ray& r) const {
	double tn, tf;
	return isIntersected(r, tn, tf);
}


bool Box::isIntersected (const Ray& r, double& tn, double& tf) const {
	Vec p1 = (min - r.orig).mapDiv(r.dir);
	Vec p2 = (max - r.orig).mapDiv(r.dir);
	tn = std::numeric_limits<double>::min();
	tf = std::numeric_limits<double>::max();

	for (int p = 0; p < 3; ++p) {
		if (r.dir[p] == 0) {//	ray is parallel
			if (r.orig[p] < min[p] || r.orig[p] > max[p])
				return false;
		} else {//	ray is not parallel
			double t1 = fmin(p1[p], p2[p]);
			double t2 = fmax(p1[p], p2[p]);

			tn = fmax(t1, tn);
			tf = fmin(t2, tf);

			if (tn > tf || tf < 0)
				return false;
		}
	}
	return true;
}


double Box::intersect (const Ray& r) const {
	double tn, tf;
	if (isIntersected(r, tn, tf))
		return r.orig.distance(r(tn));
	return 0;
}


Obj_t Box::type() const {
	return BOX;
}

// gives a bounding box for this object (itself)
Box Box::box() const {
	return *this;
}



std::string Box::toString() const {
	using std::stringstream;
	using std::endl;

	stringstream ss;
	ss << "\t" << "box" << id
		<< " [width=.5,height=1,style=filled,color=\".5 .5 .5\",shape=box,label=\"" << min << ",\\n" << max << "\"];" << endl;

	return ss.str();
}