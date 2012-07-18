enum Refl_t { DIFF,   SPEC, REFR };
enum Obj_t  { SPHERE, BOX };

/** generic object */
struct Object {
	// position, emission and color of the object
	Vec pos, emission, color;

	// reflection type (DIFFuse, SPECular, REFRactive)
	Refl_t refl;

	/** Constructor */
	Object(Vec _pos, Vec _emission, Vec _color, Refl_t _refl) :
		pos(_pos), emission(_emission), color(_color), refl(_refl) { }

	/**
	 * detects an intersection of a ray with this object
	 * returns the distance until colision (0 if nohit)
	 */
	virtual double intersect(const Ray& r) const = 0;
	virtual Obj_t  type() const = 0;
};


/** Sphere object */
struct Sphere : Object {
	// radius of the sphere
	double rad;

	/** Constructor */
	Sphere(double _rad, Vec _pos, Vec _emission, Vec _color, Refl_t _refl) :
		Object(_pos, _emission, _color, _refl), rad(_rad) { }

	/** Other */
	double intersect(const Ray& r) const {

		// Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0 
		Vec op = pos - r.orig; 
		double t;
		double eps = 1e-4;
		double b   = op.dot(r.dest);
		double det = (b * b) - op.dot(op) + (rad * rad); 

		if (det < 0)
			return 0;
		else
			det = sqrt(det); 

		return (t = b - det) > eps ? t : ((t = b + det) > eps ? t : 0); 
	}

	Obj_t type() const {
		return SPHERE;
	}
};

/** Box object */
struct Box : Object {
	// dimensions of the box
	Vec dim;

	double intersect(const Ray& r) const {
		// TODO this
		return 0;
	}

	Obj_t type() {
		return BOX;
	}
};