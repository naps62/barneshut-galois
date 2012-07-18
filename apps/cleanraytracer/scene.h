/** Scene representation */
struct Scene {
	// total size of the scene
	Vec size;
	uint width;
	uint height;

	Ray cam;
	Vec cx;
	Vec cy;

	// objects, including boxes on the borders
	vector<Sphere> objects;
	vector<Vec> img;

	Scene(uint _w, int _h, Vec _size)
	: size(_size),
	  width(_w),
	  height(_h),
	  cam(Vec(50, 52, 295.6), Vec(0, -0.042612, -1).norm()),
	  cx(width * 0.5135 / height),
	  cy((cx % cam.dest).norm() * 0.5135),
	  img(width*height) {

		initScene(size);
	}

	void raytrace(uint spp) {
		cerr << endl << fixed;
		cerr.width(2);
		cerr.precision(2);

		Vec r;

		//#pragma omp parallel for schedule(dynamic, 1) private(r) // OpenMP (you don't say?)
		#pragma omp parallel for schedule(dynamic, 1) private(r) // OpenMP (you don't say?)
		// Loop over image rows
		for (unsigned y = 0; y < height; ++y) {
			cerr << "\rRendering (" << spp << " spp, with 4 subpixels) " << (100.0 * y / (height - 1)) << '%';
			unsigned short Xi[3] = {0, 0, static_cast<unsigned short>(y*y*y)};
			// Loop cols
			for (unsigned x = 0; x < width; ++x) {
				int i = (height - y - 1) * width + x;
				// 2x2 subpixel rows
				for (int sy = 0; sy < 2; sy++) {
					// 2x2 subpixel cols
					for (int sx = 0; sx < 2; sx++, r = Vec()) {
						//	samples per pixel
						for (uint s = 0; s < spp; s++){ 
							double r1 = 2 * erand48(Xi);
							double dx = r1 < 1 ? sqrt(r1) - 1 : 1 - sqrt(2 - r1);
							double r2 = 2 * erand48(Xi);
							double dy = r2 < 1 ? sqrt(r2) - 1 : 1 - sqrt(2 - r2); 
							Vec d = cx * ( ( (sx + .5 + dx) / 2 + x) / width - .5) + 
								cy * ( ( (sy + .5 + dy) / 2 + y) / height - .5) + cam.dest; 
							r = r + radiance(Ray(cam.orig + d * 140, d.norm()), 0, Xi) * (1. / spp); 
						} // Camera rays are pushed ^^^^^ forward to start in interior
						img[i] = img[i] + Vec(clamp(r.x), clamp(r.y) , clamp(r.z)) * .25; 
					} 
				}
			}
		} 
	}

	void save(const string &file) {
		ofstream fs(file);
		fs << "P3" << endl << width << " " << height << "\n" << 255 << endl;
		for (int i=0; i < (width * height); i++) 
			fs << toInt(img[i].x) << " " << toInt(img[i].y) << " " << toInt(img[i].y) << " ";
		fs.close();
	}

	private:
	inline double clamp(double x) { return x < 0 ? 0 : x > 1 ? 1 : x; } 

	inline int toInt(double x) { return int(pow(clamp(x), 1 / 2.2) * 255 + .5); } 

	inline bool intersect(const Ray &r, double &t, int &id){ 
		double n = objects.size();
		double d;
		double inf = 1e20;
		t = 1e20; 

		for (uint i = n; i--;) {
			if( (d = objects[i].intersect(r)) && d < t){
				t = d;
				id = i;
			}
		}
		return t < inf; 
	}

	Vec radiance(const Ray &r, int depth, unsigned short *Xi){ 
		double t;                               // distance to intersection 
		int id = 0;                             // id of intersected object 
		if (!intersect(r, t, id))
			return Vec();// if miss, return black

		const Sphere &obj = objects[id];        // the hit object 
		Vec x = r.orig + r.dest * t;
		Vec n = (x - obj.pos).norm();
		Vec nl = n.dot(r.dest) < 0 ? n : n * -1;
		Vec f = obj.color;

		double p = f.x > f.y && f.x > f.z ? f.x : f.y > f.z ? f.y : f.z; // max refl

		if (++depth > 5) {
			if (erand48(Xi) < p)
				f = f * (1/p);
			else
				return obj.emission; //R.R. 
		}
		if (obj.refl == DIFF){                  // Ideal DIFFUSE reflection 
			double r1 = 2 * M_PI * erand48(Xi), r2 = erand48(Xi), r2s = sqrt(r2); 
			Vec w = nl, u=((fabs(w.x)>.1?Vec(0,1):Vec(1))%w).norm(), v=w%u; 
			Vec d = (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2)).norm(); 
			return obj.emission + f.mult(radiance(Ray(x,d), depth, Xi)); 
		} else if (obj.refl == SPEC)            // Ideal SPECULAR reflection 
			return obj.emission + f.mult(radiance(Ray(x,r.dest - n * 2 * n.dot(r.dest)), depth, Xi)); 
		Ray reflRay(x, r.dest - n * 2 * n.dot(r.dest));     // Ideal dielectric REFRACTION 
		bool into = n.dot(nl) > 0;                // Ray from outside going in? 
		double nc = 1, nt = 1.5, nnt = into ? nc / nt : nt / nc;
		double ddn = r.dest.dot(nl);
		double cos2t; 
		if ( (cos2t = 1 - nnt * nnt * (1 - ddn * ddn)) < 0)    // Total internal reflection 
			return obj.emission + f.mult(radiance(reflRay, depth, Xi)); 
		Vec tdir = (r.dest * nnt - n * ((into ? 1 : -1) * (ddn * nnt + sqrt(cos2t)) )).norm(); 
		double a = nt - nc;
		double b = nt + nc;
		double R0 = a * a / (b * b);
		double c = 1 - (into ? -ddn : tdir.dot(n)); 
		double Re = R0 + (1 - R0) * c * c * c * c * c;
		double Tr = 1 - Re;
		double P = .25 + .5 * Re;
		double RP = Re / P;
		double TP = Tr / (1 - P); 
		return obj.emission + f.mult(depth > 2 ? (erand48(Xi) < P ?   // Russian roulette 
				radiance(reflRay, depth, Xi) * RP : radiance(Ray(x,tdir), depth, Xi) * TP) : 
				radiance(reflRay, depth, Xi) * Re + radiance(Ray(x,tdir), depth, Xi) * Tr); 
	}

	void initScene(const Vec& size) {
		objects.push_back(Sphere(1e5,  Vec( 1e5+1,  40.8,      81.6),     Vec(),         Vec(.75,.25,.25), DIFF)); //Left
		objects.push_back(Sphere(1e5,  Vec(-1e5+99, 40.8,      81.6),     Vec(),         Vec(.25,.25,.75), DIFF)); //Rght
		objects.push_back(Sphere(1e5,  Vec(50,      40.8,      1e5),      Vec(),         Vec(.75,.75,.75), DIFF)); //Back
		objects.push_back(Sphere(1e5,  Vec(50,      40.8,     -1e5+170),  Vec(),         Vec(),            DIFF)); //Frnt
		objects.push_back(Sphere(1e5,  Vec(50,      1e5,       81.6),     Vec(),         Vec(.75,.75,.75), DIFF)); //Botm
		objects.push_back(Sphere(1e5,  Vec(50,     -1e5+81.6,  81.6),     Vec(),         Vec(.75,.75,.75), DIFF)); //Top
		objects.push_back(Sphere(16.5, Vec(27,      16.5,      47),       Vec(),         Vec(1,1,1)*.999,  SPEC)); //Mirr
		objects.push_back(Sphere(16.5, Vec(73,      16.5,      78),       Vec(),         Vec(1,1,1)*.999,  REFR)); //Glas
		objects.push_back(Sphere(600,  Vec(50,      681.6-.27, 81.6),     Vec(12,12,12), Vec(),            DIFF)); //Lite
	}
};