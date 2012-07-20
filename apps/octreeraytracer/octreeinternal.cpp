#include "octreeinternal.h"

// C++ includes
#include <utility>
using std::pair;
using std::make_pair;

using std::cerr;
using std::endl;

// OctreeInternal::OctreeInternal (Point pos) : Octree(pos) { init(); }

// OctreeInternal::OctreeInternal (BoundingBox bbox) : Octree(bbox.center()), _min(bbox.min), _max(bbox.max) {
// 	init();
// 	_dim = Point(abs(_max.x - _min.x), abs(_max.y - _min.y), abs(_max.z - _min.z));
// 	_dimhlf = _dim * 0.5;
// }

void OctreeInternal::insert(OctreeLeaf *leaf)
{
	// cerr << "Inserting " << *leaf << '\t';

	//	index of the voxel the object belongs to
	int i = index(leaf->pos);
	// cerr << "in voxel " << i << endl;

	//	if there is no child, make it a leaf node
	if (!childs[i]) {
		// cerr << "No child found, adding leaf." << endl;
		childs[i] = leaf;
	}
	//	there is a child, if it is a leaf, expand
	else if (childs[i]->isLeaf()) {
		// cerr << "Found a leaf child. Expanding." << endl;
		OctreeLeaf* ol = static_cast<OctreeLeaf*>(childs[i]);
		OctreeInternal* oi = new OctreeInternal(centroid(i), size * 0.5);
		// cerr << "Expanded\t" << *oi << endl;
		oi->insert(ol);
		oi->insert(leaf);
		childs[i] = oi;
	}
	//	there is a child but it is an internal, just recurse
	else {
		OctreeInternal* oi = static_cast<OctreeInternal*>(childs[i]);
		oi->insert(leaf);
	}
}

OctreeLeaf* OctreeInternal::intersect (const Ray &r) {
	// pair<OctreeLeaf*,double> closest = pair<OctreeLeaf*,double>(NULL, std::numeric_limits<double>::max());
	OctreeLeaf* closest;
	double mindistsq = std::numeric_limits<double>::max();
	for (int i = 0; i < 8; ++i) {
		if (!childs[i])
			continue;
		OctreeLeaf* c = childs[i]->intersect(r);
		if (!c)
			continue;
		Point ro(r.orig);
		double distsq = c->pos.dist_sq(ro);
		if (distsq < mindistsq) {
			closest = c;
			mindistsq = distsq;
		}
	}
	return closest;
}

// OctreeLeaf* OctreeInternal::shoot (Ray r) {
// 	node = 0;
// 	if (r.dir.x < 0) {
// 		r.orig.x = size.x - r.orig.x;
// 		r.dir.x = -r.dir.x;
// 		node |= 4;//	activate the third byte
// 	}
// 	if (r.dir.y < 0) {
// 		r.orig.y = size.y - r.orig.y;
// 		r.dir.y = -r.dir.y;
// 		node |= 2;//	activate the second byte
// 	}
// 	if (r.dir.x < 0) {
// 		r.orig.x = size.x - r.orig.x;
// 		r.dir.x = -r.dir.x;
// 		node |= 1;//	activate the first byte
// 	}

// 	double tx0 = (_min.x - r.orig.x) / r.dir.x;
// 	double tx1 = (_max.x - r.orig.x) / r.dir.x;
// 	double ty0 = (_min.y - r.orig.y) / r.dir.y;
// 	double ty1 = (_max.y - r.orig.y) / r.dir.y;
// 	double tz0 = (_min.z - r.orig.z) / r.dir.z;
// 	double tz1 = (_max.z - r.orig.z) / r.dir.z;

// 	if (max(tx0, ty0, tz0) < min(tx1, ty1, tz1))
// 		return intersect(tx0, ty0, tz0, tx1, ty1, tz1);
// 	else
// 		return NULL;
// }

// OctreeLeaf* OctreeInternal::intersect (double tx0, double ty0, double tz0, double tx1, double ty1, double tz1)
// {
// 	// if (tx1 < 0 || ty1 < 0 || tz1 < 0)
// 	// 	return NULL;

// 	// double txm = .5 * (tx0 + tx1);
// 	// double tym = .5 * (ty0 + ty1);
// 	// double tzm = .5 * (tz0 + tz1);

// 	// OctreeLeaf* first = NULL;
// 	// int current = first_node(tx0, ty0, tz0, txm, tym, tzm);
// 	// do {
// 	// 	switch (current)
// 	// 	{
// 	// 		case 0:
// 	// 			first = childs[node]->intersect(tx0, ty0, tz0, txm, tym, tzm);
// 	// 			current = new_node(txm, 4, tym, 2, tzm, 1);
// 	// 			break;
// 	// 		case 1:
// 	// 			first = childs[node ^ 1]->intersect(tx0, ty0, tzm, txm, tym, tz1);
// 	// 			current = new_node(txm, 5, tym, 3, tz1, 8);
// 	// 			break;
// 	// 		case 2:
// 	// 			first = childs[node ^ 2]->intersect(tx0, tym, tz0, txm, ty1, tzm);
// 	// 			current = new_node(txm, 6, ty1, 8, tzm, 3);
// 	// 			break;
// 	// 		case 3:
// 	// 			first = childs[node ^ 3]->intersect(tx0, tym, tzm, txm, ty1, tz1);
// 	// 			current = new_node(txm, 7, ty1, 8, tz1, 8);
// 	// 			break;
// 	// 		case 4:
// 	// 			first = childs[node ^ 4]->intersect(txm, ty0, tz0, tx1, tym, tzm);
// 	// 			current = new_node(tx1, 8, tym, 6, tzm, 5);
// 	// 			break;
// 	// 		case 5:
// 	// 			first = childs[node ^ 5]->intersect(txm, ty0, tzm, tx1, tym, tz1);
// 	// 			current = new_node(tx1, 8, tym, 7, tz1, 8);
// 	// 			break;
// 	// 		case 6:
// 	// 			first = childs[node ^ 6]->intersect(txm, tym, tz0, tx1, ty1, tzm);
// 	// 			current = new_node(tx1, 8, ty1, 8, tzm, 7);
// 	// 			break;
// 	// 		case 7:
// 	// 			first = childs[node ^ 7]->intersect(txm, tym, tzm, tx1, ty1, tz1);
// 	// 			current = 8;
// 	// 			break;
// 	// 	}
// 	// } while (!first && current < 8);

// 	// return first;
// 	return NULL;
// }

// int first_node (double tx0, double ty0, double tz0, double txm, double tym, double tzm)
// {
// 	//	Table 2: plane
// 	char plane;
// 	//	0 -> YZ
// 	//	1 -> XZ
// 	//	2 -> XY
// 	if (tx0 > ty0)
// 		if (tx0 > tz0)
// 			plane = 0;
// 		else
// 			plane = 2;
// 	else
// 		if (ty0 > tz0)
// 			plane = 1;
// 		else
// 			plane = 2;

// 	//	Table 1: first node index
// 	switch (plane)
// 	{
// 		case 0:
// 			return (txm < tz0) | ((tym < tz0) << 1);
// 		case 1:
// 			return (txm < ty0) | ((tzm < ty0) << 2);
// 		case 2:
// 			return ((tym < tx0) << 1) | ((tzm < tx0) << 2);
// 		default://	just to shut up the compiler
// 			return NULL;
// 	}
// }