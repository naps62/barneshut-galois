#ifndef ___POINT___
#define ___POINT___

//  C++ includes
#include <iostream>

//  C includes
#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "vec.h"

struct Point {
  double x, y, z;
  
  Point();
  Point(double _x, double _y, double _z);
  explicit Point(double v);
  Point(const Point* p) : x(p->x), y(p->y), z(p->z) { }

  Point(const Vec& v) : x(v.x), y(v.y), z(v.z) {}

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
    switch (index) {
      case 0: return x;
      case 1: return y;
      case 2: return z;
    }
    assert(false && "index out of bounds");
    abort();
  }

  bool operator==(const Point& other) {
    if (x == other.x && y == other.y && z == other.z)
      return true;
    return false;
  }

  bool operator!=(const Point& other) {
    return !operator==(other);
  }

  Point& operator+=(const Point& other) {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
  }

  Point& operator-=(const Point& p) {
    x -= p.x;
    y -= p.y;
    z -= p.z;
    return *this;
  }

  Point& operator*=(double value) {
    x *= value;
    y *= value;
    z *= value;
    return *this;
  }

  Point operator-(Point& p) const {
    Point r(this);
    r -= p;
    return r;
  }

  Point operator*(double value) const {
    Point p(this);
    p *= value;
    return p;
  }

  // compute euclidean distance for this point
  double dist_sq() {
    return x*x + y*y + z*z;
  }

  double dist_sq(Point& p) const { return (*this - p).dist_sq(); }

  double dist() {
    return sqrt(dist_sq());
  }

  double dist(Point& p) const { return (*this - p).dist(); }
  // double dist(Point p) { return (*this - p).dist(); }
};



/////////////////////////////////////
//  Definitions
//
inline
Point::Point() : x(0.0), y(0.0), z(0.0) { }

inline
Point::Point(double _x, double _y, double _z) : x(_x), y(_y), z(_z) { }

inline
Point::Point(double v) : x(v), y(v), z(v) { }

inline
std::ostream& operator<<(std::ostream& os, const Point& p) {
  os << "(" << p[0] << "," << p[1] << "," << p[2] << ")";
  return os;
}

#endif//___POINT___