/**
 * Sphere
 */
struct Sphere { 
	// radius
	double rad;

	// position, emission and color
	Vec pos, emission, color;

	// reflection type (DIFFuse, SPECular, REFRactive)
	Refl_t refl;

	/**
	 * Constructors
	 */
	Sphere(double rad_, Vec _pos, Vec _emission, Vec _color, Refl_t _refl) : 
		rad(_rad), pos(_pos), emission(_emission), color(_color), refl(_refl) { } 

	/**
	 * Other
	 */

	// returns distance, 0 if nohit 
	double intersect(const Ray &r) const {

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
}; 
