#include "Ray.h"

/**
 * Constructors
 */
Ray::Ray()
:	orig(),
	dir(),
	val(),
	weight(),
	valid(true)
{ }


Ray::Ray(Vec _orig, Vec _dir, Vec _val, Vec _weight, bool _valid)
:	orig(_orig),
	dir(_dir),
	val(_val),
	weight(_weight),
	valid(_valid)
{ }


/**
 * Methods
 */
Vec Ray::operator() (double t) const {
	return orig + dir * t;
}

void Ray::weightedAdd(const Vec& f) {
	val += weight.mult(f);
}

/**
 * Output
 */
std::ostream& operator<<(std::ostream& os, const Ray& r) {
	return os << "Ray:(orig: " << r.orig << ", dir: " << r.dir << ")";
}