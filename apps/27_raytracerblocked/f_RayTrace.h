#include <iostream>

#include "Completeness.h"

/**
 * Functor
 *
 * Compute radiance for all rays
 */
struct RayTrace {
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

	// global config reference
	const Config& config;

	// contribution of each sample to the pixel
	const double contrib;

	// lock on the pixel value
	GaloisRuntime::LL::SimpleLock<true>& lock;

	/**
	 * Constructor
	 */
	RayTrace(const Camera& _cam, const BVHTree* _tree, Image& _img, Pixel& _pixel, const Config& _config, GaloisRuntime::LL::SimpleLock<true>& _lock)
	:	cam(_cam),
		tree(_tree),
		img(_img),
		pixel(_pixel),
		config(_config),
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

		Vec rad;
		
		/** To blockalize:
		  *    generate a block of rays and call radiance for each one after size is met
		  *    probably each block can have a common Xi?
		  */
		for(uint sample = block.first; sample < block.second; ++sample) {
			// instead of computing radiance
			// insert ray into a block (use the same Xi for the entire block, and compare results)
			Ray ray = generateRay(pixel, Xi);
			rad    += radiance(ray, 0, Xi) * contrib;
		}


		

		completeness.lock.lock();
		pixel += Vec(clamp(rad.x), clamp(rad.y), clamp(rad.z));
		//std::cerr << "\rRendering (" << config.spp * 4 << " spp) " << (100.0 * ++completeness.val / (img.size())) << '%';
		completeness.lock.unlock();
		//updateStatus();
	}


	private:
	// generate a new primary ray
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


	void updateStatus() {
		completeness.lock.lock();
		std::cerr << "\rRendering (" << config.spp * 4 << " spp) " << (100.0 * ++completeness.val / (img.size())) << '%';
		completeness.lock.unlock();
	}

	/** compute total radiance for a ray */
	/** To blockalize:
	 *     receive a block of rays rather than a single one */
	// receive a block of rays
	Vec radiance(const Ray &ray, uint depth, unsigned short *Xi){ 
		// distance to intersection 
		double dist;

		// id of intersected object 
		Object* obj_ptr;

		// if miss, return black
		if (!tree->intersect(ray, dist, obj_ptr))
			return Vec();

		// the hit object 
		const Sphere &obj = *static_cast<Sphere*>(obj_ptr);
		Vec f = obj.color;

		//Russian Roullete to stop
		if (++depth > config.maxdepth) {
			double max_refl = obj.color.max_coord();
			if (erand48(Xi) < max_refl)
				f *= (1 / max_refl);
			else
				return obj.emission;
		}

		Vec innerResult;

		Vec hit_point = ray.orig + ray.dir * dist;
		Vec norm      = (hit_point - obj.pos).norm();
		Vec nl        = norm.dot(ray.dir) < 0 ? norm : (norm * -1);

		/** To blockalize:
		 *     each case generates a new ray except REFR which can generate 2.
		 *     Insert each generated ray into new block and recurse on it. Then add the result to each of the previous rays
		 *     Return value is common to every case, so no problem there */
		Ray childRay;
		double weight = 1.0f;
		switch(obj.refl) {
			case DIFF: // Ideal DIFFUSE reflection
				childRay = computeDiffuseRay(Xi, hit_point, nl);
				break;

			case SPEC: // Ideal SPECULAR reflection
				childRay = computeSpecularRay(hit_point, norm, ray.dir);
				break;

			case REFR: // Ideal dielectric REFRACTION
				childRay = computeRefractedRay(Xi, ray, norm, nl, hit_point, weight);
				break;

			default:
				assert(false && "Invalid object reflection type");
				abort();
		}

		return obj.emission + f.mult(radiance(childRay, depth, Xi) * weight);
	}

	/**
	 * Sub-ray calculation
	 */

	// creates a diffuse ray
	Ray computeDiffuseRay(ushort *Xi, Vec& hit_point, Vec& nl) const {
		double r1  = erand48(Xi) * 2 * M_PI;
		double r2  = erand48(Xi);
		double r2s = sqrt(r2);

		Ray result;

		Vec w   = nl;
		Vec u   = ((fabs(w.x) > 0.1 ? Vec((double)0, 1) : Vec(1)) % w).norm();
		Vec v   = w % u;
		Vec dir = (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2)).norm(); 

		result.orig = hit_point;
		result.dir = dir;

		return result;
	}

	// creates a specular ray
	Ray computeSpecularRay(const Vec& hit_point, const Vec& norm, const Vec& r_dir) const {
		Ray result(hit_point, r_dir - norm * 2 * norm.dot(r_dir));
		return result;
	}

	// creates a reflected ray
	Ray computeReflectedRay(const Vec& orig, const Vec& dir) const {
		Ray result(orig, dir);
		return result;
	}

	// creates a refracted ray
	Ray computeRefractedRay(ushort *Xi, const Ray& ray, const Vec& norm, const Vec& nl, const Vec& hit_point, double& weight) const {
		const double nc    = 1;
		const double nt    = 1.5;
		const bool   into  = norm.dot(nl) > 0;
		const double nnt   = into ? nc / nt : nt / nc;
		const double ddn   = ray.dir.dot(nl);
		const double cos2t = 1 - nnt * nnt * (1 - ddn * ddn);

		Ray result;

		// Total internal reflection
		if (cos2t < 0) {
			result = computeSpecularRay(hit_point, norm, ray.dir);
			weight = 1.0;
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
				result = computeSpecularRay(hit_point, norm, ray.dir);
				weight = Re / P;
			}
			else {
				result = computeReflectedRay(hit_point, tdir);
				weight = Tr / (1 - P);
			}

			//} else { // otherwise, sum both
			//	refrResult = radiance(reflRay, depth, Xi) * Re + radiance(refrRay, depth, Xi) * Tr;
			//}
		}
		return result;
	}
};