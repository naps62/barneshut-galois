




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

	// random number generators
	std::vector<RNG>& rngs;

	PrimaryRayGen(const Camera& _cam, const Image& _img, const Pixel& _pixel, RayList& _rays, std::vector<RNG>& _rngs)
		:	cam(_cam),
			img(_img),
			pixel(_pixel),
			rays(_rays),
			rngs(_rngs)
		{ }

	/**
	 * Functor
	 */
	template<typename Context>
	void operator()(BlockDef* _block, Context&) {
		BlockDef& block = *_block;

		for(uint s = block.first; s < block.second; ++s) {
			Ray& ray = *(rays[s]);
			generateRay(ray, pixel, rngs[GaloisRuntime::LL::getTID()]);
		}	
	}

	private:

	void generateRay(Ray& ray, const Pixel& pixel, RNG& rng) const {
		double r1 = 2 * rng();
		double r2 = 2 * rng();
		double dirX = (r1 < 1) ? (sqrt(r1) - 1) : (1 - sqrt(2 - r1));
		double dirY = (r2 < 1) ? (sqrt(r2) - 1) : (1 - sqrt(2 - r2));

		Vec dir = cam.cx * ((dirX + pixel.w) / img.width - 0.5) +
					 cam.cy * ((dirY + pixel.h) / img.height - 0.5) +
					 cam.dir;
		
		ray.orig = cam.orig;
		ray.dir  = dir.norm();
		ray.val  = Vec(0.0, 0.0, 0.0);
		ray.valid = true;
		ray.weight = Vec(1.0, 1.0, 1.0);
	}
};

#endif // _PRIMARY_RAY_H