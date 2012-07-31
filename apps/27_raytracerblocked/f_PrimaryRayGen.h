




#ifndef _PRIMARY_RAY_H
#define _PRIMARY_RAY_H

#include "structs/Vec.h"

/**
 * Generates primary rays for a pixel
 */
struct PrimaryRayGen {
	// Optimize runtime for no conflict case
	typedef int tt_does_not_need_aborts;

	const Camera& cam;
	const Image& img;
	const Pixel& pixel;
	RayList& rays;

	PrimaryRayGen(const Camera& _cam, const Image& _img, const Pixel& _pixel, RayList& _rays)
		:	cam(_cam),
			img(_img),
			pixel(_pixel),
			rays(_rays)
		{ }

	/**
	 * Functor
	 */
	template<typename Context>
	void operator()(BlockDef* _block, Context&) {
		BlockDef& block = *_block;

		ushort seed = static_cast<ushort>(pixel.h * pixel.w * block.first);
		ushort Xi[3] = {0, 0, seed};

		for(uint s = block.first; s < block.second; ++s) {
			Ray& ray = *(rays[s]);
			generateRay(ray, pixel, Xi);
		}	
	}

	private:

	void generateRay(Ray& ray, const Pixel& pixel, ushort *Xi) const {
		double r1 = 2 * erand48(Xi);
		double r2 = 2 * erand48(Xi);
		double dirX = (r1 < 1) ? (sqrt(r1) - 1) : (1 - sqrt(2 - r1));
		double dirY = (r2 < 1) ? (sqrt(r2) - 1) : (1 - sqrt(2 - r2));

		Vec dir = cam.cx * ((dirX + pixel.w) / img.width - 0.5) +
					 cam.cy * ((dirY + pixel.h) / img.height - 0.5) +
					 cam.dir;
		
		ray.orig = cam.orig;
		ray.dir  = dir.norm();
	}
};

#endif // _PRIMARY_RAY_H