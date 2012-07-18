/**
 * Generic 3D vector
 */
struct Vec {
	double x, y, z;// position, also color (r,g,b)

	/**
	 * Constructors
	 */
	Vec() : x(0.0), y(0.0), z(0.0) { }
	Vec(double _x, double _y, double _z) : x(_x), y(_y), z(_z) { }
	explicit Vec(double v) : x(v), y(v), z(v) { }

	/**
	 * Operators
	 */
 	double operator[](const int index) const {
		switch (index) {
			case 0: return x;
			case 1: return y;
			case 2: return z;
		}
		assert(false && "index out of bounds");
		abort();
	}

	double& operator[](const int index) {
		switch(index) {
			case 0: return x;
			case 1: return y;
			case 2: return z;
		}
		assert(false && "index out of bounds");
		abort();
	}

	bool operator==(const Vec& other) const { return (x == other.x && y == other.y && z == other.z); }
	bool operator!=(const Vec& other) const { return !operator==(other); }

	Vec& operator+=(const Vec& o) { x += o.x; y += o.y; z += o.z; return *this; }
	Vec& operator-=(const Vec& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
	Vec& operator*=(const Vec& o) { x *= o.x; y *= o.y; z *= o.z; return *this; }

	Vec operator+(const Vec &b) const { return Vec(x + b.x, y + b.y, z + b.z); } 
	Vec operator-(const Vec &b) const { return Vec(x - b.x, y - b.y, z - b.z); } 
	Vec operator*(double b)     const { return Vec(x * b, y * b, z * b); } 
	Vec operator%(Vec &b)       const { return Vec(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x); } 
	
	/**
	 * other
	 */
	Vec mult(const Vec &b) const {
		return Vec(x*b.x, y*b.y, z*b.z);
	} 

	Vec& norm() {
		return *this = *this * (1 / sqrt(x*x + y*y + z*z));
	}

	double dot(const Vec &b) const {
		return x * b.x + y * b.y + z * b.z;
	}
}; 

std::ostream& operator<<(std::ostream& os, const Vec& p) {
  os << "(" << p.x << "," << p.y << "," << p.z << ")";
  return os;
}