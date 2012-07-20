/**	Non-leaf octree node. Represents a voxel as an object container.
 */
struct OctreeInternal : public Octree {
	Octree* childs[8];
	Point size;

	/**
	 * \param pos Centroid (current node) in cartesian coordinates.
	 */
	OctreeInternal (Point pos) : Octree(pos) {
		memset(child, NULL, sizeof(*childs * 8));
	}

	OctreeInternal (BoundingBox bbox) : Octree(bbox.center()), _min(bbox.min), _max(bbox.max) {
		memset(child, NULL, sizeof(*childs * 8));
		_dim = Point(abs(_max.x - _min.x), abs(_max.y - _min.y), abs(_max.z - _min.z));
		_dimhlf *= 0.5;
	}

	void insert(OctreeObject *obj);

	OctreeObject* shoot (Ray x);

private:
	Point _min, _max;
	Point _dim, _dimhlf;
	int _node;

	/**	Returns the index of the sub-voxel a leaf belongs in.
	 * \param p Position of the leaf.
	 */
	int index(const Point& p);

	double max(double x, double y, double z);
	double min(double x, double y, double z);
	
	//	returns the first intersected object in this node
	OctreeLeaf* intersect (double tx0, double ty0, double tz0, double tx1, double ty1, double tz1);

	//	returns the index of the first sub-node intersected by the ray
	int first_node (double tx0, double ty0, double tz0, double txm, double tym, double tzm);

	//	returns the index of the next sub-node to be intersected by the ray
	int new_node (double f1, int i1, double f2, int i2, double f3, int i3);

	// void updateCenter(Point& p, int index, double radius);

	//	returns the centroid of a sub-voxel identified by its index
	Point centroid(int index);
};



/////////////////////////////////////
//  Definitions - OctreeInternal
//
inline
double max(double x, double y, double z)
{ return (x > y) ? ((x > z) ? x : z) : ((y > z ? y : z)); }

inline
double min(double x, double y, double z)
{ return (x < y) ? ((x < z) ? x : z) : ((y < z ? y : z)); }

inline
int index(const Point& p)
{
	return
		(p.x > pos.x) * 4 +
		(p.y > pos.y) * 2 +
		(p.z > pos.z) * 1;
}

inline
int new_node (double f1, int i1, double f2, int i2, double f3, int i3)
{
	if (f1 < f2)
		if (f1 < f3)
			return i1;
	else
		if (f2 < f3)
			return i2;
	return i3;
}

inline
Point centroid (int index)
{
	Point p(pos);
	//	move in the x axis
	if (index & 4)
		p.x += dimhlf.x;
	else
		p.x -= dimhlf.x;
	//	move in the y axis
	if (index & 2)
		p.y += dimhlf.y;
	else
		p.y -= dimhlf.y;
	//	move in the z axis
	if (index & 1;
		p.z += dimhlf.z;
	else
		p.z -= dimhlf.z;
	return p;
}






// inline
// void updateCenter(Point& p, int index, double radius)
// {
//   for (int i = 0; i < 3; i++) {
//     double v = (index & (1 << i)) > 0 ? radius : -radius;
//     p[i] += v;
//   }
// }
