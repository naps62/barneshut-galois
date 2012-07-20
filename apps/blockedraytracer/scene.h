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

	Ray cam;
	Vec cx;
	Vec cy;

	// objects, including boxes on the borders
	ObjectList objects;
	Image img;
	const uint spp;
	const uint maxdepth;

	/**
	 * Constructor
	 */
	Scene(uint _w, uint _h, uint _spp, uint _maxdepth, uint n)
	: //TODO: esta merda assim é um nojo, raio de valores mais aleatórios
	  //cam(Vec(50, 52, 295.6), Vec(0, -0.042612, -1).norm()),
	  cam(Vec(0, 0, -309), Vec(0, -0.0, 1).norm()),
	  cx(_w * 0.5 / _h),
	  cy((cx % cam.dir).norm() * 0.5),
	  img(_w, _h),
	  spp(_spp),
	  maxdepth(_maxdepth) {

		initScene(n);
	}

	/**
	 * Main function
	 */
	void raytrace() {
		Galois::StatTimer T_rayTrace("RayTrace");

		Galois::setActiveThreads(numThreads);
		
		//
		// Step 1. Compute total radiance for each pixel. Each ray has a contribution of 0.25/spp to its corresponding pixel
		//
		T_rayTrace.start();
		Galois::for_each(wrap(img.begin()), wrap(img.end()), RayTrace(cam, cx, cy, objects, img, spp, maxdepth));
		T_rayTrace.stop();
	}

	/** save image to file */
	void save(const string &file) {
		ofstream fs(file);
		fs << "P3" << endl << img.width << " " << img.height << "\n" << 255 << endl;
		for (int i=0; i < (img.width * img.height); i++) 
			fs << toInt(img[i].x) << " " << toInt(img[i].y) << " " << toInt(img[i].z) << " ";
		fs.close();
	}

	/**
	 * Private methods
	 */
	private:

	/** initializes scene with some objects */
	void initScene(uint n) {
		const double side = 100;
		const double half = side / 2;

		objects.push_back(Sphere(1e5,  Vec( 1e5+half,         0,        0),  Vec(),  Vec(.75,.25,.25), DIFF)); //Left
		objects.push_back(Sphere(1e5,  Vec(-1e5-half,         0,        0),  Vec(),  Vec(.25,.25,.75), DIFF)); //Rght
		objects.push_back(Sphere(1e5,  Vec(        0,  1e5+half,        0),  Vec(),  Vec(.75,.75,.75), DIFF)); //Botm
		objects.push_back(Sphere(1e5,  Vec(        0, -1e5-half,        0),  Vec(),  Vec(.75,.75,.75), DIFF)); //Top
		objects.push_back(Sphere(1e5,  Vec(        0,         0, 1e5+half),  Vec(),  Vec(.25,.55,.25), DIFF)); //Front
		//objects.push_back(Sphere(1e5,  Vec(        0,         0, -1e5-half),  Vec(),  Vec(), DIFF)); //Frnt

		objects.push_back(Sphere(300,  Vec(0, 300+half-0.5, -50), Vec(22,22,22), Vec(1, 1, 1),    DIFF));
		objects.push_back(Sphere(2, Vec(50, 50, 50), Vec(), Vec(0,1,0), DIFF));

		const double space = 100/n;

		Refl_t TYPE=REFR;
		double cx = -50 + space/2;
		for(uint x = 0; x < n; ++x, cx+=space) {
			double cy = -50 + space/2;
			for(uint y = 0; y < n; ++y, cy+=space) {
				double cz = -100 + space/2;
				for(uint z = 0; z < n; ++z, cz+=space) {
					objects.push_back(Sphere(10, Vec(cx, cy, cz), Vec(), Vec(0.75, 0.75, 0.75), TYPE));
					TYPE = (TYPE == REFR) ? SPEC : REFR;
				}
			}
		}
	}
};