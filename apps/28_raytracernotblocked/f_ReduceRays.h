#ifndef _REDUCE_RAYS_H
#define _REDUCE_RAYS_H

#include <Galois/Accumulator.h>

/**
 * Reduces contributions of all vectors to a single Vector
 */
struct ReduceRays {
	Galois::GAccumulator<Vec>& accum;

	const double contrib;

	ReduceRays(const uint size, Galois::GAccumulator<Vec>& _accum)
		:	accum(_accum),
			contrib(1 / (double) size)
	{ }

	/**
	 * Functor
	 */
	template<typename Context>
	void operator()(Ray** ray, Context&) {
		accum.get() += (*ray)->val * contrib;
	}
};

#endif // _REDUCE_RAYS_H