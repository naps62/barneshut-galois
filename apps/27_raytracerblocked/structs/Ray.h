#ifndef _RAY_H
#define _RAY_H 

#include <ostream>
#include <vector>

#include "Vec.h"

 /**
  * A single ray
  */
struct Ray {
	// vector origin and direction
	Vec orig;
	Vec dir;
	Vec val;
	Vec weight;
	bool valid;

	/**
	 * Constructors
	 */
	Ray();
	Ray(Vec _orig, Vec _dir, Vec _val = Vec(), Vec weight = Vec(1.0, 1.0, 1.0), bool _valid = true);

	Vec operator() (double t) const;

	void weightedAdd(const Vec& f);

	/**
	 * Output
	 */
	friend std::ostream& operator<<(std::ostream& os, const Ray& r);
};

typedef std::vector<Ray*> RayList;

#endif // _RAY_H