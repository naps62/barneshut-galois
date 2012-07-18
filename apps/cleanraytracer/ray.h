 /**
  * A single ray
  */
 struct Ray {
	 Vec orig, dir;

	 Ray(Vec _orig, Vec _dir) : orig(_orig), dir(_dir) { }
 }; 
 
std::ostream& operator<<(std::ostream& os, const Ray& r) {
  os << "orig: " << r.orig << ", dir: " << r.dir;
  return os;
}