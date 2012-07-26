/**
 * Bounding Box
 */
struct BoundingBox {
	Vec min;
	Vec max;

	BoundingBox() 
	:	min(true,  std::numeric_limits<double>::max()),
		max(true, -std::numeric_limits<double>::max()) { }

	BoundingBox(Vec _min, Vec _max)
	:	min(_min),
		max(_max) { }

	void containBox(const BoundingBox& box) {
		min = min.min(box.min);
		max = max.max(box.max);
	}
	
	double intersect (const Ray& r) const {
		double tn, tf;
		if (isIntersected(r, tn, tf))
			return r.orig.distance(r(tn));
		return 0;
	}

	bool isIntersected (const Ray& r) const {
		double tn, tf;
		return isIntersected(r, tn, tf);
	}

	bool isIntersected (const Ray& r, double& tn, double& tf) const {
		Vec p1 = (min - r.orig).mapDiv(r.dir);
		Vec p2 = (max - r.orig).mapDiv(r.dir);
		tn = std::numeric_limits<double>::min();
		tf = std::numeric_limits<double>::max();

		for (int p = 0; p < 3; ++p) {
			if (r.dir[p] == 0) {//	ray is parallel
				if (r.orig[p] < min[p] || r.orig[p] > max[p])
					return false;
			} else {//	ray is not parallel
				double t1 = fmin(p1[p], p2[p]);
				double t2 = fmax(p1[p], p2[p]);

				tn = fmax(t1, tn);
				tf = fmin(t2, tf);

				if (tn > tf || tf < 0)
					return false;
			}
		}

		return true;
	}
};