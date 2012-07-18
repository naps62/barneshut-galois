/** Scene representation */
struct Scene {
	// total size of the scene
	Vec size;

	// objects, including boxes on the borders
	vector<Object> objects;

	Scene(Vec _size, uint nspheres) : size(_size) {
		initScene(size);
	}

	private:
	initScene(const Vec& size) {
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