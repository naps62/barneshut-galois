#ifndef _SORTING_TRAITS_H
#define _SORTING_TRAITS_H

struct SpatialRayOriginSortingTraits {
	typedef Ray* Point_3;


	struct LessRayOrigX {
		bool operator()(const Ray* p, const Ray* q) const {
			return p->orig.x < q->orig.x;
		}
	};

	struct LessRayOrigY {
		bool operator()(const Ray* p, const Ray* q) const {
			return p->orig.y < q->orig.y;
		}
	};

	struct LessRayOrigZ {
		bool operator()(const Ray* p, const Ray* q) const {
			return p->orig.z < q->orig.z;
		}
	};

	typedef LessRayOrigX Less_x_3;
	typedef LessRayOrigY Less_y_3;
	typedef LessRayOrigZ Less_z_3;

	Less_x_3 less_x_3_object() const { return Less_x_3(); }
	Less_y_3 less_y_3_object() const { return Less_y_3(); }
	Less_z_3 less_z_3_object() const { return Less_z_3(); }
};


struct SpatialRayDirSortingTraits {
	typedef Ray* Point_3;

	struct LessRayDirX {
		bool operator()(const Ray* p, const Ray* q) const {
			return p->dir.x < q->dir.x;
		}
	};

	struct LessRayDirY {
		bool operator()(const Ray* p, const Ray* q) const {
			return p->dir.y < q->dir.y;
		}
	};

	struct LessRayDirZ {
		bool operator()(const Ray* p, const Ray* q) const {
			return p->dir.z < q->dir.z;
		}
	};

	typedef LessRayDirX Less_x_3;
	typedef LessRayDirY Less_y_3;
	typedef LessRayDirZ Less_z_3;

	Less_x_3 less_x_3_object() const { return Less_x_3(); }
	Less_y_3 less_y_3_object() const { return Less_y_3(); }
	Less_z_3 less_z_3_object() const { return Less_z_3(); }
};

#endif // _SORTING_TRAITS_H