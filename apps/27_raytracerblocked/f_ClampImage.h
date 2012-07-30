#ifndef _CLAMP_IMAGE_H
#define _CLAMP_IMAGE_H

#include "structs/Vec.h"

/**
 * Clamps given vectors to values in the [0, 1] range
 */
struct ClampImage {
	// Optimize runtime for no conflict case
	typedef int tt_does_not_need_aborts;

	/**
	 * Functor
	 */
	template<typename Context>
	void operator()(Vec* vec, Context&) {
		clamp(vec->x);
		clamp(vec->y);
		clamp(vec->z);
	}

	private:
	/** clamps a value between 0 and 1 */
	inline void clamp(double& x) {
		if (x < 0)      x = 0;
		else if (x > 1) x = 1;
	}
};

#endif // _CLAMP_IMAGE_H