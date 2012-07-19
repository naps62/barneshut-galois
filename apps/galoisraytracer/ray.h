 /**
  * A single ray
  */
struct Ray {
	Vec orig, dir;
	//unsigned short Xi[3];
	//uint pixelIdx;

	Ray() : orig(), dir() { }

	Ray(Vec _orig, Vec _dir/*, unsigned short *_Xi = NULL, uint _pixelIdx = 0*/)
		: orig(_orig),
	 	  dir(_dir)/*,
	 	  pixelIdx(_pixelIdx)*/ {

		/*if (_Xi != NULL) {
			Xi[0] = _Xi[0];
			Xi[1] = _Xi[1];
			Xi[2] = _Xi[2];
		}*/
	}
}; 

std::ostream& operator<<(std::ostream& os, const Ray& r) {
  os << "orig: " << r.orig << ", dir: " << r.dir /*<< ", pixelIdx: " << r.pixelIdx*/;
  return os;
}