#include "object.h"
#include "octree.h"

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
	vector<Sphere*> objects;
	vector<Vec> img;

	//	octree
	OctreeInternal* tree;

	/**
	 * Constructor
	 */
	Scene(uint _w, int _h, Vec _size)
	: size(_size),
	  width(_w),
	  height(_h),
	  //TODO: esta merda assim é um nojo, raio de valores mais aleatórios
	  cam(Vec(50, 52, 295.6), Vec(0, -0.042612, -1).norm()),
	  cx(width * 0.5135 / height),
	  cy((cx % cam.dir).norm() * 0.5135),
	  img(width*height) {

		initScene(size);

		//	TODO: order objects

		vector<BoundingBox> boxes;
		for (vector<Sphere*>::iterator it = objects.begin(); it < objects.end(); ++it)
			boxes.push_back((*it)->getBoundingBox());
		// BoundingBox bbox(boxes);
		// cerr << bbox << endl;
		tree = new OctreeInternal(BoundingBox(boxes));
		// cerr << *tree << endl;

		for (vector<Sphere*>::iterator it = objects.begin(); it < objects.end(); ++it)
			tree->insert(*it);

		// exit(1);
	}

	/**
	 * Destructor
	 */
	~Scene() {
		// delete tree;
	}

	/**
	 * Main function
	 */
	void raytrace(uint spp) {
		cerr << endl << fixed;
		cerr.width(2);
		cerr.precision(2);

		Vec r;

		//#pragma omp parallel for schedule(dynamic, 1) private(r) // OpenMP (you don't say?)
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
								cy * ( ( (sy + .5 + dy) / 2 + y) / height - .5) + cam.dir; 
							r = r + radiance(Ray(cam.orig + d * 140, d.norm()), 0, Xi) * (1. / spp); 
						} // Camera rays are pushed ^^^^^ forward to start in interior
						img[i] = img[i] + Vec(clamp(r.x), clamp(r.y) , clamp(r.z)) * .25; 
					} 
				}
			}
		} 
		cerr << endl;
	}

	/** save image to file */
	void save(const string &file) {
		ofstream fs(file);
		fs << "P3" << endl << width << " " << height << "\n" << 255 << endl;
		for (unsigned i = 0; i < (width * height); i++) 
			fs << toInt(img[i].x) << " " << toInt(img[i].y) << " " << toInt(img[i].y) << " ";
		fs.close();
	}

	/**
	 * Private methods
	 */
	private:

	/** clamps a value between 0 and 1 */
	inline double clamp(double x) {
		if (x < 0)      return 0;
		else if (x > 1) return 1;
		else            return x;
	} 

	/** interpolates a double to a 0-255 integer scale */
	inline int toInt(double x) {
		return int(pow(clamp(x), 1 / 2.2) * 255 + 0.5);
	} 

	/** given a ray, calc which object it intersects with */
	// inline
	// bool intersect(const Ray &r, double &dist, int &id){ 
	// 	double size = objects.size();
	// 	double inf = 1e20;
	// 	dist       = 1e20; 

	// 	for (uint i = size; i--;) {
	// 		double d = objects[i]->intersect(r);
	// 		if(d && d < dist){
	// 			dist = d;
	// 			id   = i;
	// 		}
	// 	}
	// 	return dist < inf; 
	// }

	/** compute total radiance for a ray */
	Vec radiance(const Ray &r, int depth, unsigned short *Xi){ 
		// distance to intersection 
		// double dist;

		// id of intersected object 
		// int id = 0;

		// if miss, return black
		//	TODO: here, shoot the octree
		// if (!intersect(r, dist, id))
		// 	return Vec();
		OctreeLeaf* leaf;
		if ( !(leaf = tree->intersect(r)) )
			return Vec();

		Sphere* obj = static_cast<Sphere*>(leaf);
		Point ro(r.orig);
		double dist = Point(obj->pos).dist(ro);

		// the hit object 
		// const Sphere* obj = objects[id];
		Vec f         = obj->color;

		//Russian Roullete to stop
		if (++depth > 5) {
			// max refl
			double max_refl = obj->color.max_coord();
			if (erand48(Xi) < max_refl)
				f = f * (1 / max_refl);
			else
				return obj->emission; 
		}


		Vec hit_point = r.orig + r.dir * dist;
		Vec norm      = (hit_point - obj->pos).norm();
		Vec nl        = norm.dot(r.dir) < 0 ? norm : (norm * -1);

		switch(obj->refl) {
			// Ideal DIFFUSE reflection
			case DIFF: {
				double r1  = 2 * M_PI * erand48(Xi);
				double r2  = erand48(Xi);
				double r2s = sqrt(r2);

				Vec w   = nl;
				Vec u   = ((fabs(w.x) > 0.1 ? Vec(0, 1) : Vec(1)) % w).norm();
				Vec v   = w % u;
				Vec dir = (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2)).norm(); 

				Ray diffuse_ray(hit_point, dir);
				return obj->emission + f.mult(radiance(diffuse_ray, depth, Xi)); 
				break;
			}

			// Ideal SPECULAR reflection
			case SPEC: {
				Ray specular_ray(hit_point,r.dir - norm * 2 * norm.dot(r.dir));
				return obj->emission + f.mult(radiance(specular_ray, depth, Xi));
				break;
			}

			// Ideal dielectric REFRACTION
			case REFR:
			default: {
				Ray reflRay(hit_point, r.dir - norm * 2 * norm.dot(r.dir));
				// Ray from outside going in?
				bool into    = norm.dot(nl) > 0;
				double nc    = 1, nt  = 1.5;
				double nnt   = into ? nc / nt : nt / nc;
				double ddn   = r.dir.dot(nl);
				double cos2t = 1 - nnt * nnt * (1 - ddn * ddn);

				// Total internal reflection
				if (cos2t < 0) {
					return obj->emission + f.mult(radiance(reflRay, depth, Xi));
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
						if (erand48(Xi) < P) refrResult = radiance(reflRay, depth, Xi) * RP;
						else                 refrResult = radiance(refrRay, depth, Xi) * TP;
					} else { // otherwise, sum both
						refrResult = radiance(reflRay, depth, Xi) * Re + radiance(refrRay, depth, Xi) * Tr;
					}
					return obj->emission + f.mult(refrResult); 
				}
			}
		}
	}

	/** initializes scene with some objects */
	void initScene(const Vec& size) {
		objects.push_back(new Sphere(1e5,  Vec( 1e5+1,  40.8,      81.6),     Vec(),         Vec(.75,.25,.25), DIFF)); //Left
		objects.push_back(new Sphere(1e5,  Vec(-1e5+99, 40.8,      81.6),     Vec(),         Vec(.25,.25,.75), DIFF)); //Rght
		objects.push_back(new Sphere(1e5,  Vec(50,      40.8,      1e5),      Vec(),         Vec(.75,.75,.75), DIFF)); //Back
		objects.push_back(new Sphere(1e5,  Vec(50,      40.8,     -1e5+170),  Vec(),         Vec(),            DIFF)); //Frnt
		objects.push_back(new Sphere(1e5,  Vec(50,      1e5,       81.6),     Vec(),         Vec(.75,.75,.75), DIFF)); //Botm
		objects.push_back(new Sphere(1e5,  Vec(50,     -1e5+81.6,  81.6),     Vec(),         Vec(.75,.75,.75), DIFF)); //Top
		objects.push_back(new Sphere(16.5, Vec(27,      16.5,      47),       Vec(),         Vec(1,1,1)*.999,  SPEC)); //Mirr
		objects.push_back(new Sphere(16.5, Vec(73,      16.5,      78),       Vec(),         Vec(1,1,1)*.999,  REFR)); //Glas
		objects.push_back(new Sphere(600,  Vec(50,      681.6-.27, 81.6),     Vec(12,12,12), Vec(),            DIFF)); //Lite
	}
};