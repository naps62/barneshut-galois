#ifndef _OBJECT_H
#define _OBJECT_H

#include <ostream>
#include <string>

#include "Ray.h"
//#include "Box.h"

struct Box;


// reflection type of the object
enum Refl_t {
	DIFF,
	SPEC,
	REFR
};

// type of the object (unused for now)
enum Obj_t  {
	SPHERE,
	BOX
};

/**
 * generic scene object
 */
struct Object {
	// static counter for unique ID's
	static uint nextId;

	// id of this instance
	uint id;

	// position, emission and color of the object
	Vec pos;
	Vec emission;
	Vec color;

	// reflection type (DIFFuse, SPECular, REFRactive)
	Refl_t refl;


	//
	// Methods
	//

	/** Constructor */
	Object();
	Object(Vec _pos, Vec _emission, Vec _color, Refl_t _refl);
	Object(const Object& c);

	/**
	 * detects an intersection of a ray with this object
	 * returns the distance until colision (0 if nohit)
	 */
	virtual double intersect(const Ray& r) const = 0;
	virtual Obj_t type() const = 0;
	virtual Box box() const = 0;
	virtual std::string toString() const = 0;

	friend std::ostream& operator<<(std::ostream& os, const Object& o);
};

#endif //_OBJECT_H