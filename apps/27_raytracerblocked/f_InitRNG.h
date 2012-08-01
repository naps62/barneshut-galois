#ifndef _INIT_RNG_H
#define _INIT_RNG_H

/**
 * Clamps given vectors to values in the [0, 1] range
 */
struct InitRNG {
	// Optimize runtime for no conflict case
	typedef int tt_does_not_need_aborts;

	/**
	 * Functor
	 */
	template<typename Context>
	void operator()(RNG* rng, Context&) {
		rng->init(GaloisRuntime::LL::getTID() * time(NULL));
	}
};

#endif // _CLAMP_IMAGE_H