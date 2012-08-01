#include <iostream>

#include "Completeness.h"

/**
 * Functor
 *
 * Compute radiance for all rays
 */
struct CastRays {
	typedef int tt_does_not_need_stats;
	typedef int tt_does_not_need_aborts;

	// camera reference
	const Camera& cam;

	// object tree
	const BVHTree* tree;

	// result image
	Image& img;

	// current pixel being processed
	Pixel &pixel;

	// current list of rays
	RayList& rays;

	// global config reference
	const Config& config;

	// how many rays left to process
	uint rays_left;

	// contribution of each sample to the pixel
	const double contrib;

	// lock on the pixel value
	GaloisRuntime::LL::SimpleLock<true>& lock;

	/**
	 * Constructor
	 */
	CastRays(const Camera& _cam,
				const BVHTree* _tree,
				Image& _img,
				Pixel& _pixel,
				RayList& _rays,
				const Config& _config,
				uint _rays_left,
				GaloisRuntime::LL::SimpleLock<true>& _lock)
	:	cam(_cam),
		tree(_tree),
		img(_img),
		pixel(_pixel),
		rays(_rays),
		config(_config),
		rays_left(_rays_left),
		contrib(1.0 / (double) config.spp),
		lock(_lock)
	{ }

	/**
	 * Functor
	 */
	// receive a block of pixels instead
	template<typename Context>
	void operator()(BlockDef* _block, Context&) {
		BlockDef& block = *_block;


		ushort seed = static_cast<ushort>(pixel.h * pixel.w * block.first);
		ushort Xi[3] = {0, 0, seed};
		
		/** To blockalize:
		  *    generate a block of rays and call radiance for each one after size is met
		  *    probably each block can have a common Xi?
		  */
		radiance(block, 0, Xi) * contrib;
	}


	private:

	/** compute total radiance for a ray */
	/** To blockalize:
	 *     receive a block of rays rather than a single one */
	// receive a block of rays
	Vec radiance(const BlockDef& block/*const Ray &ray*/, uint depth, unsigned short *Xi){
		Ray** blockStart = &(rays[block.first]);
		uint blockSize = block.second - block.first;

		typedef std::map<Ray*,std::pair<double, Object*> > ColisionList;
		ColisionList colisions;

		uint rays_disabled = 0;

		// distance to intersection 
		double dist;

		// id of intersected object
		Object* obj_ptr;

		// if miss, return black
		if (!tree->intersect(blockStart, blockSize, colisions))
			return Vec();

		for(ColisionList::iterator it = colisions.begin(); it != colisions.end(); ++it) {
			Ray& ray          = *(it->first);
			double dist       = it->second.first;
			const Sphere& obj = *static_cast<Sphere*>(it->second.second);


			Vec f = obj.color;

			ray.weightedAdd(obj.emission);

			if (++depth > config.maxdepth && ray.valid) {
				double max_refl = obj.color.max_coord();
				if (erand48(Xi) < max_refl)
					f *= (1 / max_refl);
				else {
					ray.valid = false;
					rays_disabled++;
				}
			}

			if (ray.valid == false)
				continue;

			Vec innerResult;
			Vec hit_point = ray.orig + ray.dir * dist;
			Vec norm      = (hit_point - obj.pos).norm();
			Vec nl        = norm.dot(ray.dir) < 0 ? norm : (norm * -1);

			switch(obj.refl) {
				case DIFF: // Ideal DIFFUSE reflection
					computeDiffuseRay(ray, Xi, hit_point, nl);
					break;

				case SPEC: // Ideal SPECULAR reflection
					computeSpecularRay(ray, hit_point, norm, ray.dir);
					break;

				case REFR: // Ideal dielectric REFRACTION
					computeRefractedRay(ray, Xi, norm, nl, hit_point);
					break;

				default:
					assert(false && "Invalid object reflection type");
					abort();
			}

			ray.weight *= f;
		}

		lock.lock();
		rays_left -= rays_disabled;
		lock.unlock();
	}

	/**
	 * Sub-ray calculation
	 */

	// creates a diffuse ray
	void computeDiffuseRay(Ray& ray, ushort *Xi, Vec& hit_point, Vec& nl) const {
		double r1  = erand48(Xi) * 2 * M_PI;
		double r2  = erand48(Xi);
		double r2s = sqrt(r2);

		Vec w   = nl;
		Vec u   = ((fabs(w.x) > 0.1 ? Vec((double)0, 1) : Vec(1)) % w).norm();
		Vec v   = w % u;
		Vec dir = (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2)).norm(); 

		ray.orig = hit_point;
		ray.dir  = dir;
	}

	// creates a specular ray
	void computeSpecularRay(Ray& ray, const Vec& hit_point, const Vec& norm, const Vec& r_dir) const {
		ray.orig = hit_point;
		ray.dir  = r_dir - norm * 2 * norm.dot(r_dir);
	}

	// creates a reflected ray
	void computeReflectedRay(Ray& ray, const Vec& orig, const Vec& dir) const {
		ray.orig = orig;
		ray.dir  = dir;
	}

	// creates a refracted ray
	void computeRefractedRay(Ray& ray, ushort *Xi, const Vec& norm, const Vec& nl, const Vec& hit_point) const {
		const double nc    = 1;
		const double nt    = 1.5;
		const bool   into  = norm.dot(nl) > 0;
		const double nnt   = into ? nc / nt : nt / nc;
		const double ddn   = ray.dir.dot(nl);
		const double cos2t = 1 - nnt * nnt * (1 - ddn * ddn);
		double weight = 1.0;

		// Total internal reflection
		if (cos2t < 0) {
			computeSpecularRay(ray, hit_point, norm, ray.dir);
		} else {
			Vec tdir = (ray.dir * nnt - norm * ((into ? 1 : -1) * (ddn * nnt + sqrt(cos2t)) )).norm();
			double a = nt - nc;
			double b = nt + nc;
			double R0 = a * a / (b * b);
			double c = 1 - (into ? -ddn : tdir.dot(norm));
			double Re = R0 + (1 - R0) * c * c * c * c * c;
			double Tr = 1 - Re; 

			//if (depth > 2)

			double P = 0.25 + 0.5 * Re;
			if (erand48(Xi) < P) {
				computeSpecularRay(ray, hit_point, norm, ray.dir);
				weight = Re / P;
			}
			else {
				computeReflectedRay(ray, hit_point, tdir);
				weight = Tr / (1 - P);
			}

			//} else { // otherwise, sum both
			//	refrResult = radiance(reflRay, depth, Xi) * Re + radiance(refrRay, depth, Xi) * Tr;
			//}
		}
		
		ray.weight *= weight;
	}
};