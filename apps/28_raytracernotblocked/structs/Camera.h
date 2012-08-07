#ifndef _CAMERA_H
#define _CAMERA_H

#include "Ray.h"

struct Camera : public Ray {
	Vec cx;
	Vec cy;

	/**
	 * Constructors
	 */
	Camera(Vec _pos, Vec _dir, uint w, uint h);
};

#endif // _CAMERA_H