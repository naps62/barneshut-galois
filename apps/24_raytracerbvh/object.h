enum Refl_t { DIFF,   SPEC, REFR };
enum Obj_t  { SPHERE, BOX };

/**
 * generic object
 */
struct Object {
	// position, emission and color of the object
	Vec pos, emission, color;

	// reflection type (DIFFuse, SPECular, REFRactive)
	Refl_t refl;
	uint id;

	/** Constructor */
	Object(Vec _pos, Vec _emission, Vec _color, Refl_t _refl)
		:	pos(_pos),
			emission(_emission),
			color(_color),
			refl(_refl) { }

	Object(const Object& c)
		:	pos(c.pos),
			emission(c.emission),
			color(c.color),
			refl(c.refl),
			id(c.id) { }

	/**
	 * detects an intersection of a ray with this object
	 * returns the distance until colision (0 if nohit)
	 */
	virtual double intersect(const Ray& r) const = 0;
	virtual Obj_t  type() const = 0;
	virtual BoundingBox box() const = 0;
};

/**
 * Sphere object
 */
struct Sphere : public Object {
	// radius of the sphere
	double rad;

	/** Constructor */
	Sphere(double _rad, Vec _pos, Vec _emission, Vec _color, Refl_t _refl)
		:	Object(_pos, _emission, _color, _refl),
			rad(_rad) { }

	Sphere(const Sphere& c)
		:	Object(c),
			rad(c.rad) { }

	/** Other */
	double intersect(const Ray& r) const {

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

	Obj_t type() const {
		return SPHERE;
	}

	BoundingBox box() const {
		return BoundingBox(pos-rad, pos+rad);
	}
};

std::ostream& operator<<(std::ostream& os, const Sphere p) {
	BoundingBox box = p.box();

	os << "\t" << "b" << p.id << " [width=.5,height=1,style=filled,color=\".5 .5 .5\",shape=box,label=\"" << box.min << ",\\n" << box.max << "\"];" << endl
		<< "\t" << "b" << p.id << " -> " << "o" << p.id << endl
		<< "\t" << "o" << p.id <<  " [label=\"(" << p.rad << ",\\n" << p.pos << ")\"];" << endl;
		
	

	return os;
}

/**
 * Box object
 */
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

