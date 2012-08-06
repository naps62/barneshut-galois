#ifndef _SPHERE_H
#define _SPHERE_H

#include <string>
#include <ostream>

#include "Object.h"
#include "Box.h"
#include "Ray.h"

/**
 * Sphere object
 */
struct Sphere : public Object {
	// radius of the sphere
	double rad;

	//
	// Methods
	//

	/**
	 * Constructors
	 */
	Sphere(	const double _rad,
				const Vec _pos,
				const Vec _emission,
				const Vec _color,
				const Refl_t _refl);
	Sphere(const Sphere& c);

	/**
	 * Inherited
	 */
	double intersect(const Ray& r) const;
	Obj_t type() const;

	/**
	 * Other
	 */
	Box box() const;

	// output
	std::string toString() const;
};

#endif // _SPHERE_H