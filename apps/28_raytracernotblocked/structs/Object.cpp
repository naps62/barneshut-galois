#include "Object.h"

uint Object::nextId = 0;

/**
 * Constructors
 */
Object::Object()
:	id(nextId++),
	pos(),
	emission(),
	color(),
	refl()
{ }

Object::Object(Vec _pos, Vec _emission, Vec _color, Refl_t _refl) 
:	id(nextId++),
	pos(_pos),
	emission(_emission),
	color(_color),
	refl(_refl)
{ }

Object::Object(const Object& c)
:	id(c.id),
	pos(c.pos),
	emission(c.emission),
	color(c.color),
	refl(c.refl)
{ }


std::ostream& operator<<(std::ostream& os, Object& o) {
	os << o.toString();
	return os;
}

/**
 * Comparators
 */
ObjectComp_Axis::ObjectComp_Axis(const ushort _axis)
:	axis(_axis)
{ }

bool ObjectComp_Axis::operator() (const Object* o1, const Object* o2) {
	return o1->pos[axis] < o2->pos[axis];
}