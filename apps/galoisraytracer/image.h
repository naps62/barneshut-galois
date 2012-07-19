/**
 * Pixel
 */
struct Pixel : public Vec {
	uint w, h;

	/**
	 * Constructors
	 */
	Pixel(uint _w=0, uint _h=0, double _x=0, double _y=0, double _z=0)
	: Vec(_x, _y, _z), w(_w), h(_h) { }

	
}; 

std::ostream& operator<<(std::ostream& os, const Pixel& p) {
	os << "(" << p.w << ", " << p.h << ")"
		<< " (r:" << p.x << ", g:" << p.y << ", b:" << p.z << ")";
	return os;
}

/**
 * Image
 */
struct Image : vector<Pixel> {
	uint width, height;

	/**
	 * Constructors
	 */
	Image(uint _width, uint _height) 
	:	vector<Pixel>(_width*_height),
		width(_width),
		height(_height) {

			initImage();
		}

	void initImage() {
		uint index = 0;
		for(uint h = height; h; --h) {
			for(uint w = 0; w < width; ++w, ++index) {
				(*this)[index].h = h;
				(*this)[index].w = w;
				(*this)[index].x = 0;
				(*this)[index].y = 0;
				(*this)[index].z = 0;
			}
		}
	}
};