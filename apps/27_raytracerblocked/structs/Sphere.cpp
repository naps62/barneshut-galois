#include <sstream>

#include "Sphere.h"

/**
 * Constructors
 */
Sphere::Sphere(double _rad, Vec _pos, Vec _emission, Vec _color, Refl_t _refl)
:	Object(_pos, _emission, _color, _refl),
	rad(_rad)
{ }


Sphere::Sphere(const Sphere& c)
:	Object(c),
	rad(c.rad)
{ }


/**
 * Methods
 */
double Sphere::intersect(const Ray& r) const {
	// Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0 
	Vec op = pos - r.orig; 
	double t;
	double eps = 1e-4;
	double b   = op.dot(r.dir);
	double det = (b * b) - op.dot(op) + (rad * rad); 

	if (det < 0)
		return 0;
	else
		det = sqrt(det); 

	return (t = b - det) > eps ? t : ((t = b + det) > eps ? t : 0); 
}


Obj_t Sphere::type() const {
	return SPHERE;
}


Box Sphere::box() const {
	return Box(pos-rad, pos+rad);
}

std::string Sphere::toString() const {
	using std::stringstream;
	using std::endl;

	Box box = this->box();

	stringstream ss;
	ss << "\t" << "b" << id
		<< " [width=.5,height=1,style=filled,color=\".5 .5 .5\",label=\"" << box.min << ",\\n" << box.max << "\"];" << endl
		<< "\t" << "b" << id << " -> " << "sphere" << id << endl
		<< "\t" << "sphere" << id <<  " [label=\"(" << rad << ",\\n" << pos << ")\"];" << endl;

	return ss.str();
}