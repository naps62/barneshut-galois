#include "Pixel.h"

Pixel::Pixel(	uint _w,
					uint _h,
					double _x,
					double _y,
					double _z)
:	Vec(_x, _y, _z),
	w(_w),
	h(_h)
{ }

void Pixel::setColor(const Vec& c) {
	this->x = c.x;
	this->y = c.y;
	this->z = c.z;
}

std::ostream& operator<<(std::ostream& os, const Pixel& p) {
	os << "(" << p.w << ", " << p.h << ")"
		<< " (r:" << p.x << ", g:" << p.y << ", b:" << p.z << ")";
	return os;
}