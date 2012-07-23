/**
 * Bounding Box
 */
struct BoundingBox {
	Vec min;
	Vec max;

	BoundingBox() 
	:	min(true, std::numeric_limits<double>::max()),
		max(true, std::numeric_limits<double>::min()) { }

	BoundingBox(Vec _min, Vec _max)
	:	min(_min),
		max(_max) { }

	void containBox(const BoundingBox& box) {
		min = min.min(box.min);
		max = max.max(box.max);
	}
};