template<typename T>
struct Deref : public std::unary_function<T, T*> {
	T* operator()(T& item) const { return &item; }
};

boost::transform_iterator<Deref<Ray>, Rays::iterator> 
wrap(Rays::iterator it) {
	return boost::make_transform_iterator(it, Deref<Ray>());
}

boost::transform_iterator<Deref<Pixel>, Image::iterator>
wrap(Image::iterator it) {
	return boost::make_transform_iterator(it, Deref<Pixel>());
}


/** Scene representation */
struct Scene {
	// total size of the scene
	Vec size;

	Ray cam;
	Vec cx;
	Vec cy;

	// objects, including boxes on the borders
	ObjectList objects;
	Image img;

	const int spp;
	Rays rays;

	/**
	 * Constructor
	 */
	Scene(uint _w, int _h, Vec _size, uint _spp)
	: size(_size),
	  //TODO: esta merda assim é um nojo, raio de valores mais aleatórios
	  cam(Vec(50, 52, 295.6), Vec(0, -0.042612, -1).norm()),
	  cx(_w * 0.5135 / _h),
	  cy((cx % cam.dir).norm() * 0.5135),
	  img(_w, _h),
	  spp(_spp),
	  rays(0) {

		initScene(size);
	}

	/**
	 * Main function
	 */
	void raytrace() {
		Galois::StatTimer T("Raytracing");
		Galois::StatTimer T_rayGen("RayGeneration");
		Galois::StatTimer T_rayTrace("RayTrace");
		Galois::StatTimer T_imgClamp("ImgClamp");

		Galois::setActiveThreads(numThreads);

		T.start();
		
		//
		// Step 2. Compute total radiance for each pixel. Each ray has a contribution of 0.25/spp to its corresponding pixel
		//
		T_rayTrace.start();
		Galois::for_each(wrap(img.begin()), wrap(img.end()), RayTrace(cam, cx, cy, objects, img, spp));
		//for(unsigned int i = 0; i < rays.size(); ++i) {
		//	Ray& ray = rays[i];
		//	img[ray.pixelIdx] += radiance(ray, 0) * (0.25 / spp);
		//}
		T_rayTrace.stop();


		//
		// Step 3. Clamp all values to a 0-255 scale
		//
		T_imgClamp.start();
		Galois::for_each(wrap(img.begin()), wrap(img.end()), ClampImage());
		//for(unsigned int i = 0; i < img.size(); ++i) {
		//	Vec& rad = img[i];
		//	img[i] = Vec(clamp(img[i].x), clamp(img[i].y), clamp(img[i].z));
		//}
		T_imgClamp.stop();

		T.stop();
	}

	/** save image to file */
	void save(const string &file) {
		ofstream fs(file);
		fs << "P3" << endl << img.width << " " << img.height << "\n" << 255 << endl;
		for (int i=0; i < (img.width * img.height); i++) 
			fs << toInt(img[i].x) << " " << toInt(img[i].y) << " " << toInt(img[i].y) << " ";
		fs.close();
	}

	/**
	 * Private methods
	 */
	private:

	/*void generateRays() {
		for (unsigned y = 0; y < height; ++y) {						// Loop rows
			unsigned short Xi[3] = {0, 0, static_cast<unsigned short>(y*y*y)};
			for (unsigned x = 0; x < width; ++x) {						// Look cols
				int pixelIdx = (height - y - 1) * width + x;
				for (int sy = 0; sy < 2; sy++) {							// 2x2 subpixel rows
					for (int sx = 0; sx < 2; sx++) {						// 2x2 subpixel cols
						for (uint s = 0; s < spp; s++){ 					// samples

							double r1 = 2 * erand48(Xi);
							double r2 = 2 * erand48(Xi);

							double dirX = r1 < 1 ? sqrt(r1) - 1 : 1 - sqrt(2 - r1);
							double dirY = r2 < 1 ? sqrt(r2) - 1 : 1 - sqrt(2 - r2); 

							Vec dir = cx * (((sx + 0.5 + dirX)/2 + x) / width  - 0.5) + 
										 cy * (((sy + 0.5 + dirY)/2 + y) / height - 0.5) +
										 cam.dir; 

							Vec pos = cam.orig + dir * 140;

							// Camera rays are pushed forward to start in interior
							rays.push_back(Ray(pos, dir.norm(), Xi, pixelIdx));
						}
					}
				}
			}
		}
	}*/

	/** initializes scene with some objects */
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