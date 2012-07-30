#ifndef _RAY_H
#define _RAY_H 

#include <ostream>

#include "Vec.h"

 /**
  * A single ray
  */
struct Ray {
	// vector origin and direction
	Vec orig;
	Vec dir;

	// father id (-1 if no father)
	int father;

	/**
	 * Constructors
	 */
	Ray();
	Ray(Vec _orig, Vec _dir);

	Vec operator() (double t) const;

	/**
	 * Output
	 */
	friend std::ostream& operator<<(std::ostream& os, const Ray& r);
};

#endif // _RAY_H