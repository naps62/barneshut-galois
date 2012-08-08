#ifndef ___F_REDUCE_BOXES_H___
#define ___F_REDUCE_BOXES_H___

#include "BoundingBox.h"

namespace Barneshut {

/**
 * Functor
 *
 * Merges each body's position with current bounding box to generate the global bounding box
 */
struct ReduceBoxes {
	// NB: only correct when run sequentially or tree-like reduction
	typedef int tt_does_not_need_stats;
	BoundingBox& initial;

	ReduceBoxes(BoundingBox& _initial): initial(_initial) { }

	template<typename Context>
		void operator()(Body* b, Context&) {
			initial.merge(b->pos);
		}
};

}

#endif//___F_REDUCE_BOXES_H___
