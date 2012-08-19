#ifndef ___CGAL_H___
#define ___CGAL_H___

// local includes
#include "point.h"

struct PointSpatialSortingTraits {
	typedef Point<3>* Point_3;

	typedef struct {
		bool operator() (const Point<3>* const p, const Point<3>* const q) const { return (*p)[0] < (*q)[0]; }
	} Less_x_3;

	typedef struct {
		bool operator() (const Point<3>* const p, const Point<3>* const q) const { return (*p)[1] < (*q)[1]; }
	} Less_y_3;

	typedef struct {
		bool operator() (const Point<3>* const p, const Point<3>* const q) const { return (*p)[2] < (*q)[2]; }
	} Less_z_3;

	Less_x_3 less_x_3_object() const { return Less_x_3(); }
	Less_y_3 less_y_3_object() const { return Less_y_3(); }
	Less_z_3 less_z_3_object() const { return Less_z_3(); }
};

#endif//___CGAL_H___