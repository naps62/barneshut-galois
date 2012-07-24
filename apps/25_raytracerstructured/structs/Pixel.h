#ifndef _PIXEL_H
#define _PIXEL_H

#include "Vec.h"

struct Pixel : public Vec {
	// coords on the image
	uint w;
	uint h;

	/**
	 * Constructors
	 */
	Pixel(uint _w=0,
			uint _h=0,
			double _x=0,
			double _y=0,
			double _z=0);

	friend std::ostream& operator<<(std::ostream& os, const Pixel& p);
};

#endif // _PIXEL_H