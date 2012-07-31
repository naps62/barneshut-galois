




#ifndef _PRIMARY_RAY_H
#define _PRIMARY_RAY_H

#include "structs/Vec.h"

/**
 * Generates primary rays for a pixel
 */
struct PrimaryRayGen {
	// Optimize runtime for no conflict case
	typedef int tt_does_not_need_aborts;

	const Image& img;
	const Pixel& pixel;
	RayList& rays;

	PrimaryRayGen(Image& _img, Pixel& _pixel, RayList& _rays)
		:	img(_img),
			pixel(_pixel),
			rays(_rays)
		{ }

	/**
	 * Functor
	 */
	template<typename Context>
	void operator()(BlockDef* _block, Context&) {
		Pixel& pixel = *pix;
		BlockDef& block = *_block;

		ushort seed = static_cast<ushort>(pixel.h * pixel.w * block.start);
		ushort Xi[3] = {0, 0, seed};

		for(uint s = block.first; s < block.second; ++s) {
			Ray& ray = rays[s];
			ray = generateRay(pixel, Xi);
		}	
	}

	private:

	Ray generateRay(const Pixel& pixel, ushort *Xi) const {
		double r1 = 2 * erand48(Xi);
		double r2 = 2 * erand48(Xi);
		double dirX = (r1 < 1) ? (sqrt(r1) - 1) : (1 - sqrt(2 - r1));
		double dirY = (r2 < 1) ? (sqrt(r2) - 1) : (1 - sqrt(2 - r2));

		Vec dir = cam.cx * ((dirX + pixel.w) / img.width - 0.5) +
					 cam.cy * ((dirY + pixel.h) / img.height - 0.5) +
					 cam.dir;
		
		Ray result(cam.orig, dir.norm());
		return result;
	}
};

#endif // _PRIMARY_RAY_H