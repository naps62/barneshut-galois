#ifndef ___OBJECT___
#define ___OBJECT___

#include "boundingbox.h"
#include "octreeleaf.h"
#include "ray.h"
#include "vec.h"

enum Refl_t { DIFF,   SPEC, REFR };
enum Obj_t  { SPHERE, BOX };

/** generic object */
struct Object : public OctreeLeaf {
	// position, emission and color of the object
	Vec pos, emission, color;

	// reflection type (DIFFuse, SPECular, REFRactive)
	Refl_t refl;

	/** Constructor */
	Object(Vec _pos, Vec _emission, Vec _color, Refl_t _refl) :
		OctreeLeaf(Point(_pos.x, _pos.y, _pos.z)),
		pos(_pos), emission(_emission), color(_color), refl(_refl) { }

	/**
	 * detects an intersection of a ray with this object
	 * returns the distance until colision (0 if nohit)
	 */
	// virtual double intersect(const Ray& r) const = 0;
	// virtual OctreeLeaf* intersect(const Ray& r) = 0;
	virtual Obj_t  type() const = 0;
	virtual BoundingBox getBoundingBox() const = 0;
};


/** Sphere object */
struct Sphere : public Object {
	// radius of the sphere
	double rad;

	/** Constructor */
	Sphere(double _rad, Vec _pos, Vec _emission, Vec _color, Refl_t _refl) :
		Object(_pos, _emission, _color, _refl), rad(_rad) { }

	Obj_t type() const {
		return SPHERE;
	}

	OctreeLeaf* intersect(const Ray& r) {
		if (intersect_dst(r) < 1e20) {
			return this;
		} else
			return NULL;
	}

	BoundingBox getBoundingBox() const
	{
		Point min(pos.x - rad, pos.y - rad, pos.z - rad);
		Point max(pos.x + rad, pos.y + rad, pos.z + rad);
		return BoundingBox(min,max);
	}

private:
	/** Other */
	double intersect_dst(const Ray& r) const {

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
};

inline
std::ostream& operator<< (std::ostream& out, Sphere& s) {
	out << s.pos;
	return out;
}

/** Box object */
// struct Box : Object {
// 	// dimensions of the box
// 	Vec dim;

// 	double intersect(const Ray& r) const {
// 		// TODO this
// 		return 0;
// 	}

// 	Obj_t type() {
// 		return BOX;
// 	}

// 	BoundingBox getBoundingBox() const { return BoundingBox(); }
// };

#endif//___OBJECT___
