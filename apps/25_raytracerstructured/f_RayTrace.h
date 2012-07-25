struct Completeness {
	GaloisRuntime::LL::SimpleLock<true> lock;
	uint val;
};


/**
 * Functor
 *
 * Compute radiance for all rays
 */
struct RayTrace {
	typedef int tt_does_not_need_stats;
	typedef int tt_does_not_need_aborts;

	const Ray& cam;
	const Vec& cx;
	const Vec& cy;
	const ObjectList& objects;
	const BVHTree* tree;
	Image& img;
	const uint spp;
	const double contrib;
	const uint maxdepth;
	Completeness& completeness;

	RayTrace(const Ray& _cam, const Vec& _cx, const Vec& _cy, const ObjectList& _objects, const BVHTree* _tree, Image& _img, const uint _spp, const uint _maxdepth, Completeness& _completeness)
		: cam(_cam),
		  cx(_cx),
		  cy(_cy),
		  objects(_objects),
		  tree(_tree),
		  img(_img),
		  spp(_spp),
		  contrib(1.0 / spp),
		  maxdepth(_maxdepth),
		  completeness(_completeness) { }

	// receive a block of pixels instead
	template<typename Context>
	void operator()(Pixel* p, Context&) {
		Pixel& pixel = *p;
		unsigned short seed = static_cast<unsigned short>(pixel.h * pixel.h * pixel.w);
		unsigned short Xi[3] = {0, 0, seed};

		Vec rad;
		
		/** To blockalize:
		  *    generate a block of rays and call radiance for each one after size is met
		       probably each block can have a common Xi? */
		radiance(Ray(Vec(0, 0, 0), Vec(0, 0, 1)), 0, Xi);
		for(uint sy = 0; sy < 2; ++sy) {
			for(uint sx = 0; sx < 2; ++sx, rad = Vec()) {
				for(uint sample = 0; sample < spp; ++sample) {
					double r1 = 2 * erand48(Xi);
					double r2 = 2 * erand48(Xi);
					double dirX = r1 < 1 ? sqrt(r1) - 1 : 1 - sqrt(2 - r1);
					double dirY = r2 < 1 ? sqrt(r2) - 1 : 1 - sqrt(2 - r2); 

					Vec dir = cx * (((sx + 0.5 + dirX)/2 + pixel.w) / img.width  - 0.5) + 
							 	 cy * (((sy + 0.5 + dirY)/2 + pixel.h) / img.height - 0.5) +
							 	 cam.dir; 
					Vec pos = cam.orig + dir * 140;

					// instead of computing radiance, insert ray into a block (use the same Xi for the entire block, and compare results)
					rad += radiance(Ray(pos, dir.norm()), 0, Xi) * contrib;

				}
				pixel += Vec(clamp(rad.x), clamp(rad.y), clamp(rad.z)) * 0.25;
			}
		}

		completeness.lock.lock();
			cerr << "\rRendering (" << spp * 4 << " spp) " << (100.0 * ++completeness.val / (img.size())) << '%';
		completeness.lock.unlock();
	}

	private:
	/** compute total radiance for a ray */
	/** To blockalize:
	 *     receive a block of rays rather than a single one */
	// receive a block of rays
	Vec radiance(const Ray &r, uint depth, unsigned short *Xi){ 
		// distance to intersection 
		double dist;

		// id of intersected object 
		Object* obj_ptr = 0;

		// if miss, return black
		if (!tree->intersect(r, dist, obj_ptr))
			return Vec();

		// the hit object 
		const Sphere &obj = *static_cast<Sphere*>(obj_ptr);
		Vec f             = obj.color;

		//Russian Roullete to stop
		if (++depth > maxdepth) {
			// max refl
			double max_refl = obj.color.max_coord();
			if (erand48(Xi) < max_refl)
				f = f * (1 / max_refl);
			else
				return obj.emission; 
		}


		Vec hit_point = r.orig + r.dir * dist;
		Vec norm      = (hit_point - obj.pos).norm();
		Vec nl        = norm.dot(r.dir) < 0 ? norm : (norm * -1);

		/** To blockalize:
		 *     each case generates a new ray except REFR which can generate 2.
		 *     Insert each generated ray into new block and recurse on it. Then add the result to each of the previous rays
		 *     Return value is common to every case, so no problem there */
		switch(obj.refl) {
			// Ideal DIFFUSE reflection
			case DIFF: {
				double r1  = 2 * M_PI * erand48(Xi);
				double r2  = erand48(Xi);
				double r2s = sqrt(r2);

				Vec w   = nl;
				Vec u   = ((fabs(w.x) > 0.1 ? Vec((double)0, 1) : Vec(1)) % w).norm();
				Vec v   = w % u;
				Vec dir = (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2)).norm(); 

				Ray diffuse_ray(hit_point, dir);
				return obj.emission + f.mult(radiance(diffuse_ray, depth, Xi)); 
				break;
			}

			// Ideal SPECULAR reflection
			case SPEC: {
				Ray specular_ray(hit_point,r.dir - norm * 2 * norm.dot(r.dir));
				return obj.emission + f.mult(radiance(specular_ray, depth, Xi));
				break;
			}

			// Ideal dielectric REFRACTION
			case REFR:
			default: {
				Ray reflRay(hit_point, r.dir - norm * 2 * norm.dot(r.dir));
				// Ray from outside going in?
				bool into    = norm.dot(nl) > 0;
				double nc    = 1;
				double nt    = 1.5;
				double nnt   = into ? nc / nt : nt / nc;
				double ddn   = r.dir.dot(nl);
				double cos2t = 1 - nnt * nnt * (1 - ddn * ddn);

				// Total internal reflection
				if (cos2t < 0) {
					return obj.emission + f.mult(radiance(reflRay, depth, Xi));
				} else {
					Vec tdir = (r.dir * nnt - norm * ((into ? 1 : -1) * (ddn * nnt + sqrt(cos2t)) )).norm();
					double a = nt - nc;
					double b = nt + nc;
					double R0 = a * a / (b * b);
					double c = 1 - (into ? -ddn : tdir.dot(norm));
					double Re = R0 + (1 - R0) * c * c * c * c * c;
					double Tr = 1 - Re;
					double P = 0.25 + 0.5 * Re;
					double RP = Re / P;
					double TP = Tr / (1 - P); 

					// Russian roulette 
					Ray refrRay(hit_point, tdir);
					Vec refrResult;

					if (depth > 2) { // if ray is deep enough, consider only one of the contributions
						// what about blocks? try to generate always a single ray, and compare results
						if (erand48(Xi) < P) refrResult = radiance(reflRay, depth, Xi) * RP;
						else                 refrResult = radiance(refrRay, depth, Xi) * TP;
					} else { // otherwise, sum both
						refrResult = radiance(reflRay, depth, Xi) * Re + radiance(refrRay, depth, Xi) * Tr;
					}
					return obj.emission + f.mult(refrResult); 
				}
			}
		}
	}
};