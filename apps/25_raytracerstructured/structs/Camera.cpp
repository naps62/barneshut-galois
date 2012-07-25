#include "Camera.h"

Camera::Camera(Vec _pos, Vec _dir, uint w, uint h)
:	Ray(_pos, _dir),
	cx(w * 0.5 / h),
	cy((cx % this->dir).norm() * 0.5)
{ }