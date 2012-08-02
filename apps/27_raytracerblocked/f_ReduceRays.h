#ifndef _REDUCE_RAYS_H
#define _REDUCE_RAYS_H

#include <Galois/Accumulator.h>

/**
 * Reduces contributions of all vectors to a single Vector
 */
struct ReduceRays {

	RayList& rays;
	Galois::GAccumulator<Vec>& accum;

	ReduceRays(RayList& _rays, Galois::GAccumulator<Vec>& _accum)
		:	rays(_rays),
			accum(_accum)
	{ }

	/**
	 * Functor
	 */
	template<typename Context>
	void operator()(BlockDef* _block, Context&) {
		BlockDef& block = *_block;

		for(uint s = block.first; s < block.second; ++s) {
			accum.get() += rays[s]->val;
		}
	}
};

#endif // _REDUCE_RAYS_H