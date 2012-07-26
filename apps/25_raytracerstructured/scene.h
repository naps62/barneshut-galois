#include <vector>
#include <fstream>

/**
 * TODO fix this stuff
 */

template<typename T>
struct Deref : public std::unary_function<T, T*> {
	T* operator()(T& item) const { return &item; }
};

boost::transform_iterator<Deref<Ray>, Rays::iterator> 
wrap(Rays::iterator it) {
	return boost::make_transform_iterator(it, Deref<Ray>());
}

boost::transform_iterator<Deref<Pixel>, std::vector<Pixel>::iterator>
wrap(std::vector<Pixel>::iterator it) {
	return boost::make_transform_iterator(it, Deref<Pixel>());
}

/** Scene representation */
struct Scene {

	const Config& config;
	const Camera cam;

	// objects, including boxes on the borders
	ObjectList objects;
	BVHTree* root;

	Image img;

	Completeness completeness;


	/**
	 * Constructor
	 */
	Scene(const Config& _config)
	:	//TODO: esta merda assim é um nojo, raio de valores mais aleatórios
		//cam(Vec(50, 52, 295.6), Vec(0, -0.042612, -1).norm()),
		config(_config),
		cam(Vec(0, 0, -260), Vec(0, -0.0, 1).norm(), _config.w, _config.h),
		img(_config.w, _config.h) {

	  	completeness.val = 0;
		initScene(config.n);

		if (config.dump)
			root->dumpDot(std::cout);
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
		Galois::for_each(wrap(img.pixels.begin()), wrap(img.pixels.end()), RayTrace(cam, root, img, config, completeness));
		T_rayTrace.stop();
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

	/** initializes scene with some objects */
	void initScene(uint n) {
		allocSpheres(n);
		root = new BVHTree(objects);
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