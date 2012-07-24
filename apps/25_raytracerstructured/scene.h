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

boost::transform_iterator<Deref<Pixel>, vector<Pixel>::iterator>
wrap(vector<Pixel>::iterator it) {
	return boost::make_transform_iterator(it, Deref<Pixel>());
}

/** Scene representation */
struct Scene {

	Ray cam;
	Vec cx;
	Vec cy;

	// objects, including boxes on the borders
	ObjectList objects;
	BVHNode* root;

	Image img;
	const uint spp;
	const uint maxdepth;

	Completeness completeness;


	/**
	 * Constructor
	 */
	Scene(uint _w, uint _h, uint _spp, uint _maxdepth, uint n, bool dump=false)
	: //TODO: esta merda assim é um nojo, raio de valores mais aleatórios
	  //cam(Vec(50, 52, 295.6), Vec(0, -0.042612, -1).norm()),
	  cam(Vec(0, 0, -260), Vec(0, -0.0, 1).norm()),
	  cx(_w * 0.5 / _h),
	  cy((cx % cam.dir).norm() * 0.5),
	  img(_w, _h),
	  spp(_spp),
	  maxdepth(_maxdepth) {

	  	completeness.val = 0;
		initScene(n);

		if (dump)
			root->dumpDot(cout);
		//cout << root->intersect(Ray(Vec(0, 0, -1e2), Vec(0, 0, 1e6))) << endl;
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
		Galois::for_each(wrap(img.pixels.begin()), wrap(img.pixels.end()), RayTrace(cam, cx, cy, objects, root, img, spp, maxdepth, completeness));
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
		allocSpheres(n);
		root = buildBVHNode();
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
		/*objects.push_back(new Sphere(5,  Vec(0, 0, 0), Vec(22,22,22), Vec(1, 1, 1),    DIFF));
		objects.push_back(new Sphere(5,  Vec(10, 10, 10), Vec(22,22,22), Vec(1, 1, 1),    DIFF));
		objects.push_back(new Sphere(5,  Vec(-10, -10, -10), Vec(22,22,22), Vec(1, 1, 1),    DIFF));
		objects.push_back(new Sphere(5,  Vec(-10, 10, 10), Vec(22,22,22), Vec(1, 1, 1),    DIFF));*/

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

	BVHNode* buildBVHNode() {
		vector<Object*> tmp_objects;
		root = buildBVHNode(NULL, tmp_objects, 0, objects.size() - 1, 0);

		for(uint i = 0; i < objects.size(); ++i) {
			delete objects[i];
			objects[i] = tmp_objects[i];
			objects[i]->id = i;
		}
		objects = tmp_objects;
		return root;
	}

	// comparator
	class ObjectComp {
		public:
		const ushort axis;
		ObjectComp(ushort _axis) : axis(_axis) { }

		bool operator() (const Object* o1, const Object* o2) {
			return o1->pos[axis] < o2->pos[axis];
		}
	};

	void sortObjects(const uint start, const uint end, const uint axis) {
		// insert into an aux vector
		vector<Object*> sorted;
		for(uint i = start; i <= end; ++i) {
			sorted.push_back(objects[i]);
		}

		// sort it
		sort(sorted.begin(), sorted.end(), ObjectComp(axis));

		// replace chunch of original vector
		uint i = start;
		for(vector<Object*>::iterator it = sorted.begin(); it != sorted.end(); ++it, ++i) {
			objects[i] = *it;
		}
	}

	BVHNode* buildBVHNode(BVHNode* parent, vector<Object*>& new_objects, uint start, uint end, uint axis) {
		int count = end - start + 1;
		BVHNode *newNode = new BVHNode(parent);
		newNode->parent = parent;

		if (count <= 2) {
			Object* o1 = new Sphere(*static_cast<Sphere*>(objects[start]));
			new_objects.push_back(o1);
			
		
			newNode->leaf = true;
			newNode->childs[0] = o1;
		
			// update node bounding box
			newNode->updateTreeBox(o1->box());

			if (count == 2) {
				Object* o2 = new Sphere(*static_cast<Sphere*>(objects[start + 1]));
				new_objects.push_back(o2);
				newNode->updateTreeBox(o2->box());
				newNode->childs[1] = o2;
			} else {
				newNode->childs[1] = NULL;
			}
		} else {
			sortObjects(start, end, axis);
			uint center = start + (int) (count * 0.5f);
			axis = (axis == 2) ? 0 : axis+1;

			newNode->leaf = false;
			newNode->childs[0] = buildBVHNode(newNode, new_objects, start, center-1, axis);
			newNode->childs[1] = buildBVHNode(newNode, new_objects, center, end, axis);
		}

		return newNode;
	}
};