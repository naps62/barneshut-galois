#ifndef ___CGAL_H___
#define ___CGAL_H___

#include "Point.h"

namespace Barneshut {
	//  Spatial sorting CGAL stuff
	struct LessBodyX {
		bool operator()(const Body &p, const Body &q) const {
			return p.pos.x < q.pos.x;
		}
	};

	struct LessBodyY {
		bool operator()(const Body &p, const Body &q) const {
			return p.pos.y < q.pos.y;
		}
	};

	struct LessBodyZ {
		bool operator()(const Body &p, const Body &q) const {
			return p.pos.z < q.pos.z;
		}
	};

	struct SpatialBodySortingTraits {
		typedef Body Point_3;

		typedef LessBodyX Less_x_3;
		typedef LessBodyY Less_y_3;
		typedef LessBodyZ Less_z_3;

		Less_x_3 less_x_3_object() const { return Less_x_3(); }
		Less_y_3 less_y_3_object() const { return Less_y_3(); }
		Less_z_3 less_z_3_object() const { return Less_z_3(); }
	};
}

#endif//___CGAL_H___
