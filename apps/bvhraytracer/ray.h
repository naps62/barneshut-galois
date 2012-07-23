 /**
  * A single ray
  */
struct Ray {
	Vec orig, dir;
	int father;

	Ray() : orig(), dir() { father = -1; }

	Ray(Vec _orig, Vec _dir)
		: orig(_orig),
	 	  dir(_dir) {

	 	  father = -1;

	}
}; 

std::ostream& operator<<(std::ostream& os, const Ray& r) {
  os << "orig: " << r.orig << ", dir: " << r.dir /*<< ", pixelIdx: " << r.pixelIdx*/;
  return os;
}