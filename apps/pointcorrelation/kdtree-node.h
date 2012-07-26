#ifndef ___KD_TREE_NODE_H___
#define ___KD_TREE_NODE_H___

// C++ includes
#include <limits>

#include "boundingbox.h"
#include "point.h"

#ifdef _DEBUG
#include <iostream>
using std::cerr;
using std::endl;
#endif

template<unsigned K>
struct KdTreeNode {
	static unsigned counter;

	const unsigned id;
	// unsigned count;
	BoundingBox<K> box;
	Point<K>* point;
	KdTreeNode<K>* left;
	KdTreeNode<K>* right;

	KdTreeNode (vector<Point<K>*> points, int depth)
	: id(KdTreeNode<K>::counter++)
	// , count(1)
	// , box(std::numeric_limits<double>::max(), std::numeric_limits<double>::min())
	, left(NULL)
	, right(NULL)
	{
		if (points.size() > 1) {
			//	find axis for ordering
			int axis = depth % K;

			//	sort along the chosen axis
			sort(points.begin(), points.end(), Point<K>::comparator(axis));

			//	median
			unsigned median = points.size() / 2;
			point = points[median];
			while (median > 0 && points[median - 1] == point)
				point = points[--median];

			//	sub-arrays lengths
			unsigned c1 = median;
			unsigned c2 = points.size() - c1 - 1;

			//	create left node
			if (c1 > 0) {
				vector<Point<K>*> leftpoints;
				for (unsigned i = 0; i < c1; ++i)
					leftpoints.push_back(points[i]);
				left = new KdTreeNode<K>(leftpoints, depth + 1);
				box.merge(left->box);
				// count += left->count;
			} else
				left = NULL;

			//	create right node
			if (c2 > 0) {
				vector<Point<K>*> rightpoints;
				for (unsigned i = c1 + 1; i < points.size(); ++i)
					rightpoints.push_back(points[i]);
				right = new KdTreeNode<K>(rightpoints, depth + 1);
				box.merge(right->box);
				// count += right->count;
			} else
				right = NULL;

			//	add itself to the box
			box.merge(*this->point);
		} else {//	leaf
			point = points[0];
			box.merge(*point);
		}
#ifdef _DEBUG
		// cerr << *this << endl;
		// cerr << box << endl << endl;
#endif
	}


	double distanceRaised (const Point<K>& p) const { return box.minimumDistanceRaised(p); }

	unsigned correlated (const Point<K>& p, const double radrsd) const {
		if (distanceRaised(p) > radrsd)
			return 0;

		//	result variable, initialize as 1 if this node is in range
		unsigned c = point->distanceRaised(p) < radrsd;

		//	add left node points
		if (left)
			c += left->correlated(p, radrsd);

		//	add right node points
		if (right)
			c += right->correlated(p, radrsd);

		return c;
	}

	unsigned correlated (const typename Point<K>::Block& b, const double radrsd) const {
		unsigned count = 0;
		typename Point<K>::Block next;

		//	count points in block within radius
		for (unsigned i = 0; i < b.size(); ++i) {
			if (distanceRaised(*b[i]) > radrsd)
				continue;
			next.push_back(b[i]);
			count += point->distanceRaised(*b[i]) < radrsd;
		}

		if (next.size() == 0)
			return count;

		//	go for left nodes
		if (left)
			count += left->correlated(next, radrsd);

		//	go for right nodes
		if (right)
			count += right->correlated(next, radrsd);

		return count;
	}



	friend
	std::ostream& operator<< (std::ostream& out, KdTreeNode<K>& node) {
		out << node.id << "[label=\"" << *(node.point) << "\\n" << node.count << ',' << node.box << "\"];";
		if (node.left)
			out << node.id << "->" << node.left->id << "[label=\"left\"];" << *(node.left);
		if (node.right)
			out << node.id << "->" << node.right->id << "[label=\"right\"];" << *(node.right);
		// return out << "Hello from node " << node.id;
		return out;
	}

protected:
	KdTreeNode () : id(KdTreeNode<K>::counter++) {}
};

//	initialize instance counter as zero
template<unsigned K>
unsigned KdTreeNode<K>::counter = 0;

#endif//___KD_TREE_NODE_H___