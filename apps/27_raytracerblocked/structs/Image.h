#ifndef _IMAGE_H
#define _IMAGE_H	

#include <vector>

#include "Pixel.h"

/**
 * Image
 */
struct Image {
	//private:
	// the pixels
	std::vector<Pixel> pixels;

	public:
	// the dimensions
	uint width;
	uint height;

	/**
	 * Constructors
	 */
	Image(uint _width, uint _height);

	/**
	 * Pixel access
	 */
	inline const Pixel& operator[] (uint elem) const      { return pixels[elem]; }
	inline       Pixel& operator[] (uint elem)            { return pixels[elem]; }
	inline const Pixel& operator() (uint x, uint y) const { return pixels[y * width + x]; }
	inline       Pixel& operator() (uint x, uint y)       { return pixels[y * width + x]; }

	inline uint size() const { return pixels.size(); }

	private:
	void initImage();
};

#endif // _IMAGE_H