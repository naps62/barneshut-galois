#ifndef _SPATIAL_SORT_BLOCKS_H
#define _SPATIAL_SORT_BLOCKS_H

#include <CGAL/spatial_sort.h>
#include "sorting_traits.h"

/**
 * Clamps given vectors to values in the [0, 1] range
 */
struct SpatialSortBlocks {
	// Optimize runtime for no conflict case
	typedef int tt_does_not_need_aborts;

	SpatialRayDirSortingTraits sort_dir_traits;

	RayList& rays;

	SpatialSortBlocks(RayList& _rays)
		: rays(_rays) { }

	/**
	 * Functor
	 */
	template<typename Context>
	void operator()(BlockDef* _b, Context&) {
		BlockDef& block = *_b;

		RayList::iterator begin = rays.begin() + block.first;
		RayList::iterator end = rays.begin() + block.second;

		CGAL::spatial_sort(begin, end, sort_dir_traits);
	}
};

#endif // _SPATIAL_SORT_BLOCKS_H