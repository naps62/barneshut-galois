#ifndef ___RAY___
#define ___RAY___

#include "vec.h"

/**
 * A single ray
 */
struct Ray {
	Vec orig;//<	The origin of the ray.
	Vec dir;//<	The direction of the ray.
	
	Ray(Vec _orig, Vec _dir) : orig(_orig), dir(_dir) { }
}; 

#endif//___RAY___
