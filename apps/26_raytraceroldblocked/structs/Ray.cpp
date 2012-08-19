#include "Ray.h"

/**
 * Constructors
 */
Ray::Ray()
:	orig(),
	dir()
{
	father = -1;
}


Ray::Ray(Vec _orig, Vec _dir)
:	orig(_orig),
	dir(_dir)
{
	father = -1;
}


/*Ray::Ray(Ray& c)
:	orig(c.orig),
	dir(c.dir),
	father(c.father)
{ }*/


/**
 * Methods
 */
Vec Ray::operator() (double t) const {
	return orig + dir * t;
}


/**
 * Output
 */
std::ostream& operator<<(std::ostream& os, const Ray& r) {
	return os << "Ray:(orig: " << r.orig << ", dir: " << r.dir << ")";
}