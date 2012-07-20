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
	const uint spp;
	const uint maxdepth;

	/**
	 * Constructor
	 */
	Scene(uint _w, int _h, Vec _size, uint _spp, uint _maxdepth)
	: size(_size),
	  //TODO: esta merda assim é um nojo, raio de valores mais aleatórios
	  cam(Vec(50, 52, 295.6), Vec(0, -0.042612, -1).norm()),
	  cx(_w * 0.5135 / _h),
	  cy((cx % cam.dir).norm() * 0.5135),
	  img(_w, _h),
	  spp(_spp),
	  maxdepth(_maxdepth) {

		initScene(size);
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
			fs << toInt(img[i].x) << " " << toInt(img[i].y) << " " << toInt(img[i].y) << " ";
		fs.close();
	}

	/**
	 * Private methods
	 */
	private:

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