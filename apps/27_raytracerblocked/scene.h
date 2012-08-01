#ifndef _SCENE_H
#define _SCENE_H

#include <vector>
#include <fstream>

#include <CGAL/spatial_sort.h>
#include "sorting_traits.h"

/**
 * TODO fix this stuff
 */

template<typename T>
struct Deref : public std::unary_function<T, T*> {
	T* operator()(T& item) const { return &item; }
};

/*boost::transform_iterator<Deref<Ray>, Rays::iterator>  wrap(Rays::iterator it) {
	return boost::make_transform_iterator(it, Deref<Ray>());
}*/

boost::transform_iterator<Deref<Pixel>, std::vector<Pixel>::iterator>
wrap(std::vector<Pixel>::iterator it) {
	return boost::make_transform_iterator(it, Deref<Pixel>());
}

boost::transform_iterator<Deref<BlockDef>, BlockList::iterator>
wrap(BlockList::iterator it) {
	return boost::make_transform_iterator(it, Deref<BlockDef>());
}

boost::transform_iterator<Deref<RNG>, std::vector<RNG>::iterator>
wrap(std::vector<RNG>::iterator it) {
	return boost::make_transform_iterator(it, Deref<RNG>());
}

/** Scene representation */
struct Scene {

	// global config reference
	const Config& config;

	// camera
	const Camera cam;

	// objects, including boxes on the borders
	ObjectList objects;

	// 
	BVHTree* tree;

	Image img;


	/**
	 * Constructor
	 */
	Scene(const Config& _config)
	:	//TODO: esta merda assim é um nojo, raio de valores mais aleatórios
		//cam(Vec(50, 52, 295.6), Vec(0, -0.042612, -1).norm()),
		config(_config),
		cam(Vec(0, 0, -260), Vec(0, -0.0, 1).norm(), _config.w, _config.h),
		img(_config.w, _config.h) {

		this->initScene(config.n);

		if (config.dump)
			tree->dumpDot(std::cout);
	}

	/**
	 * Main function
	 */
	void raytrace() {
		Galois::StatTimer T_primaryRayGen("PrimaryRayGen");
		Galois::StatTimer T_rayTrace("RayTrace");
		Galois::setActiveThreads(numThreads);

		RayList rays(config.spp);
		BlockList blocks;
		GaloisRuntime::LL::SimpleLock<true> lock;
		SpatialRayOriginSortingTraits sort_origin_traits;
		vector<RNG> rngs(numThreads);

		Galois::for_each(wrap(rngs.begin()), wrap(rngs.end()), InitRNG());

		//
		// 1. Index rays into blocks
		//
		calcBlock(blocks, config.block, rays.size());

		//
		// 2. Allocate rays.
		//    malloc() is serialized, so no need for for_each here
		// 
		for(uint sample = 0; sample < rays.size(); ++sample) {
			rays[sample] = new Ray();	
		}
		
		//
		// 3. Main loop - for each pixel
		//
		for(uint p = 0; p < img.size(); ++p) {
			Pixel& pixel = img.pixels[p];
			uint rays_left = rays.size();

			//
			// 3.1. Compute primary ray directions
			//
			T_primaryRayGen.start();
			Galois::for_each(wrap(blocks.begin()), wrap(blocks.end()), PrimaryRayGen(cam, img, pixel, rays));
			T_primaryRayGen.stop();

			//
			// 3.2. While there are rays to compute
			//
			T_rayTrace.start();
			uint depth = 0;
			while(rays_left) {

				//
				// 3.2.1. Globally sort all rays
				//
				CGAL::spatial_sort(rays.begin(), rays.end(), sort_origin_traits);

				//
				// 2.3.2. Locally sort each block of rays
				//
				Galois::for_each(wrap(blocks.begin()), wrap(blocks.end()), SpatialSortBlocks(rays));

				//
				// 2.3.3. 
				//
				Galois::for_each(wrap(blocks.begin()), wrap(blocks.end()), CastRays(cam, tree, img, pixel, rays, config, rays_left, depth, rngs, lock));
				if (depth > 3)rays_left--;
				depth++;

				// TODO add each ray contribution to some vector
			}
			T_rayTrace.stop();

			//
			// TODO reduce the vector to get final pixel value
			//

			std::cerr << "\rRendering (" << config.spp * 4 << " spp) " << (100.0 * ++p / (img.size())) << '%';
		}
	}

	/** save image to file */
	void save() {
		std::ofstream fs(config.outfile);
		fs << "P3" << std::endl << img.width << " " << img.height << "\n" << 255 << std::endl;
		for (int i=0; i < (img.width * img.height); i++) 
			fs << toInt(img[i].x) << " " << toInt(img[i].y) << " " << toInt(img[i].z) << " ";
		fs.close();
	}

	/**
	 * Private methods
	 */
	private:

	// generates a list of index pairs, to group samples into blocks
	void calcBlock(BlockList& blocks, uint block_size, uint total) {
		uint start = 0;
		uint end = block_size;

		while(start < total) {
			blocks.push_back(BlockDef(start, end));

			start = end;
			end += block_size;
			if (end > total)
				end = total;
		}
	}

	/** initializes scene with some objects */
	void initScene(uint n) {
		allocSpheres(n);
		tree = new BVHTree(objects);
	}

	void allocSpheres(uint n) {
		const double side = 100;
		const double half = side / 2;

		objects.push_back(new Sphere(1e5,  Vec( 1e5+half,         0,        0),  Vec(),  Vec(.75,.25,.25), DIFF)); //Left
		objects.push_back(new Sphere(1e5,  Vec(-1e5-half,         0,        0),  Vec(),  Vec(.25,.25,.75), DIFF)); //Rght
		objects.push_back(new Sphere(1e5,  Vec(        0,  1e5+half,        0),  Vec(),  Vec(.75,.75,.75), DIFF)); //Botm
		objects.push_back(new Sphere(1e5,  Vec(        0, -1e5-half,        0),  Vec(),  Vec(.75,.75,.75), DIFF)); //Top
		objects.push_back(new Sphere(1e5,  Vec(        0,         0, 1e5+half),  Vec(),  Vec(.25,.55,.25), DIFF)); //Front

		objects.push_back(new Sphere(300,  Vec(0, 300+half-0.5, -0), Vec(22,22,22), Vec(1, 1, 1),    DIFF));

		if (n == 0)
			return;

		const double space = 100/n;

		unsigned short Xi[3] = {0, 0, static_cast<unsigned short>(n)};

		Refl_t TYPE=REFR;
		double cx = -50 + space/2;
		for(uint x = 0; x < n; ++x, cx+=space) {
			double cy = -50 + space/2;
			for(uint y = 0; y < n; ++y, cy+=space) {
				double cz = -100 + space/2;
				for(uint z = 0; z < n; ++z, cz+=space) {
					double dx = erand48(Xi)*space/2 - space/4;
					double dy = erand48(Xi)*space/2 - space/4;
					double dz = erand48(Xi)*space/2 - space/4;

					double tx = erand48(Xi)*0.65;
					double ty = erand48(Xi)*0.65;
					double tz = erand48(Xi)*0.65;
					objects.push_back(new Sphere(space/5, Vec(cx+dx, cy+dy, cz+dz), Vec(), Vec(tx, ty, tz), TYPE));
					TYPE = (TYPE == REFR) ? SPEC : REFR;
				}
			}
		}
	}
};

#endif // _SCENE_H