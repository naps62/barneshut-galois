/**
 * Functor
 *
 * Compute radiance for all rays
 */
struct RayTrace {
	typedef int tt_does_not_need_stats;
	typedef int tt_does_not_need_aborts;

	const ObjectList& objects;
	Image& img;
	const double contrib;

	RayTrace(const ObjectList& _objects, Image& _img, uint spp)
		: objects(_objects),
		  img(_img),
		  contrib(0.25 / spp) { }

	template<typename Context>
	void operator()(Ray* r, Context&) {
		Ray& ray = *r;
		//cout << radiance(ray, 0) << contrib << endl;
		img[ray.pixelIdx] += radiance(ray, 0) * contrib;

		//cout << img[ray.pixelIdx] << contrib << endl;
		//for(int i = 0; i < img.size(); ++i)
//			cout << img[i] << endl;
	}

	private:
	/** compute total radiance for a ray */
	Vec radiance(Ray &r, int depth){ 
		unsigned short* Xi = r.Xi;
		// distance to intersection 
		double dist;

		// id of intersected object 
		int id = 0;

		// if miss, return black
		if (!intersect(r, dist, id))
			return Vec();

		// the hit object 
		const Sphere &obj = objects[id];
		Vec f         = obj.color;

		//Russian Roullete to stop
		if (++depth > 5) {
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

		switch(obj.refl) {
			// Ideal DIFFUSE reflection
			case DIFF: {
				double r1  = 2 * M_PI * erand48(Xi);
				double r2  = erand48(Xi);
				double r2s = sqrt(r2);

				Vec w   = nl;
				Vec u   = ((fabs(w.x) > 0.1 ? Vec(0, 1) : Vec(1)) % w).norm();
				Vec v   = w % u;
				Vec dir = (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2)).norm(); 

				Ray diffuse_ray(hit_point, dir, Xi);
				return obj.emission + f.mult(radiance(diffuse_ray, depth)); 
				break;
			}

			// Ideal SPECULAR reflection
			case SPEC: {
				Ray specular_ray(hit_point,r.dir - norm * 2 * norm.dot(r.dir), Xi);
				return obj.emission + f.mult(radiance(specular_ray, depth));
				break;
			}

			// Ideal dielectric REFRACTION
			case REFR:
			default: {
				Ray reflRay(hit_point, r.dir - norm * 2 * norm.dot(r.dir), Xi);
				// Ray from outside going in?
				bool into    = norm.dot(nl) > 0;
				double nc    = 1, nt  = 1.5;
				double nnt   = into ? nc / nt : nt / nc;
				double ddn   = r.dir.dot(nl);
				double cos2t = 1 - nnt * nnt * (1 - ddn * ddn);

				// Total internal reflection
				if (cos2t < 0) {
					return obj.emission + f.mult(radiance(reflRay, depth));
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
					Ray refrRay(hit_point, tdir, Xi);
					Vec refrResult;

					if (depth > 2) { // if ray is deep enough, consider only one of the contributions
						if (erand48(Xi) < P) refrResult = radiance(reflRay, depth) * RP;
						else                 refrResult = radiance(refrRay, depth) * TP;
					} else { // otherwise, sum both
						refrResult = radiance(reflRay, depth) * Re + radiance(refrRay, depth) * Tr;
					}
					return obj.emission + f.mult(refrResult); 
				}
			}
		}
	}

	/** given a ray, calc which object it intersects with */
	inline bool intersect(const Ray &r, double &dist, int &id){ 
		double size = objects.size();
		double inf = 1e20;
		dist       = 1e20; 

		for (uint i = size; i--;) {
			double d = objects[i].intersect(r);
			if(d && d < dist){
				dist = d;
				id   = i;
			}
		}
		return dist < inf; 
	}
};