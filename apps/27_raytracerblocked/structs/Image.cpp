#include "Image.h"

Image::Image(uint _width, uint _height) 
:	pixels(_width * _height),
	width(_width),
	height(_height)
{
	initImage();
}

void Image::initImage() {
	uint index = 0;
	for(uint h = 0; h < height; ++h) {
		for(uint w = 0; w < width; ++w, ++index) {
			pixels[index].h = h;
			pixels[index].w = w;
			pixels[index].x = 0;
			pixels[index].y = 0;
			pixels[index].z = 0;
		}
	}
}