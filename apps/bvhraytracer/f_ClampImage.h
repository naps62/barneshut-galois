/**
 * Computes the position of a body after a time step.
 */
struct ClampImage {
	// Optimize runtime for no conflict case
	typedef int tt_does_not_need_aborts;

	ClampImage() { }

	template<typename Context>
	void operator()(Vec* v, Context&) {
		Vec& vec = *v;
		clamp(vec.x);
		clamp(vec.y);
		clamp(vec.z);
	}

	/** clamps a value between 0 and 1 */
	inline void clamp(double& x) {
		if (x < 0)      x = 0;
		else if (x > 1) x = 1;
	} 
};
