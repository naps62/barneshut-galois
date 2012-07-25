#ifndef _BOX_H
#define _BOX_H

#include <string>
#include <ostream>

#include "Object.h"
#include "Vec.h"
#include "Ray.h"


/**
 * Box object
 */
struct Box : public Object {
	// dimensions of the box
	Vec min;
	Vec max;

	/**
	 * Constructors
	 */
	Box();
	Box(Vec _min, Vec _max);

	// updates bounding coordinates to contain the given box
	void containBox(const Box& box);

	// checks if give ray intersects the box
	bool isIntersected(const Ray& r) const;

	// checks intersection of a ray with the box, and returns distance (0 if no hit)
	double intersect(const Ray& r) const;

	// checks if give ray intersects the box, and returns
	bool isIntersected (const Ray& r, double& tn, double& tf) const;

	// returns type of the object (box)
	Obj_t type() const;

	// gives a bounding box for this object (itself)
	Box box() const;

	// output
	std::string toString() const;
};

#endif // _BOX_H