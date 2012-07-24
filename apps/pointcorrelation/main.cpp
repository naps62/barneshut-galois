// C++ includes
#include <algorithm>
#include <iostream>
using std::cerr;
using std::endl;
#include <sstream>
#include <string>
#include <vector>
using std::vector;

// C includes
#include <stdlib.h>
#include <string.h>







template<unsigned K>
struct Point {
	double coords[K];

	struct PointComparator {
		unsigned axis;

		PointComparator (unsigned _axis) : axis(_axis) {}

		bool operator() (const Point<K>* const p1, const Point<K>* const p2) {
			return (*p1)[axis] < (*p2)[axis];
		}

		bool operator() (const Point<K>& p1, const Point<K>& p2) {
			return p1[axis] < p2[axis];
		}
	};

	Point (double _coords[K]) {
		memcpy(coords, _coords, sizeof(double) * K);

	}

	~Point () {}

	static
	PointComparator comparator (unsigned axis) { return PointComparator(axis); }

	std::string str() const {
		std::stringstream ss;
		ss << *this;
		return ss.str();
	}


	///// Misc
	double lengthSquared () const {
		double lsq = 0.0;
		for (unsigned i = 0; i < K; ++i)
			lsq += coords[i] * coords[i];
		return lsq;
	}

	double distanceSquared (const Point& p) const {
		Point q(*this);
		q -= p;
		return q.lengthSquared();
	}


	///// Operators
	Point<K>& operator-= (const Point<K>& p) {
		for (unsigned i = 0; i < K; ++i)
			coords[i] -= p.coords[i];
		return *this;
	}


	double operator[] (unsigned index) const { return coords[index]; }


	friend
	std::ostream& operator<< (std::ostream& out, const Point<K>& p) {
		out << '(';
		if (K > 0) {
			out << p[0];
			for (unsigned i = 1; i < K; ++i) {
				out << ',' << p[i];
			}
		}
		out << ')';
		return out;
	}

};




















template<unsigned K>
struct KdTreeNode {
	static unsigned counter;
	const unsigned id;
	Point<K>* point;
	KdTreeNode<K>* left;
	KdTreeNode<K>* right;

	KdTreeNode (vector<Point<K>*> points, int depth) : id(KdTreeNode<K>::counter++), left(NULL), right(NULL) {
		if (points.size() > 1) {
			//	find axis for ordering
			int axis = depth % K;

			//	sort along the chosen axis
			sort(points.begin(), points.end(), Point<K>::comparator(axis));

			unsigned c1 = points.size() / 2;//	median
			unsigned c2 = points.size() - c1 - 1;

			//	set this node to be the median
			point = points[c1];

			if (c1 > 0) {
				vector<Point<K>*> leftpoints;
				for (unsigned i = 0; i < c1; ++i)
					leftpoints.push_back(points[i]);
				left = new KdTreeNode<K>(leftpoints, depth + 1);
			} else
				left = NULL;

			if (c2 > 0) {
				vector<Point<K>*> rightpoints;
				for (unsigned i = c1 + 1; i < points.size(); ++i)
					rightpoints.push_back(points[i]);
				right = new KdTreeNode<K>(rightpoints, depth + 1);
			} else
				right = NULL;
		} else
			point = points[0];
	}


	double distanceSquared (const Point<K>& p) const { return point->distanceSquared(p); }


	unsigned correlated (const Point<K>& p, const double radsq) const {
		double dsq = distanceSquared(p);
		// cerr << '\t' << *point << '\t' << dsq << endl;
		unsigned count = dsq < radsq;
		if (count) {
			if (left)
				count += left->correlated(p, radsq);
			if (right)
				count += right->correlated(p, radsq);
		} else {
			if (left && left->distanceSquared(p) < dsq)
				count += left->correlated(p, radsq);
			if (right && right->distanceSquared(p) < dsq)
				count += right->correlated(p, radsq);
		}
		return count;
	}



	friend
	std::ostream& operator<< (std::ostream& out, KdTreeNode<K>& node) {
		out << node.id << "[label=\"" << *(node.point) << "\"];";
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



















template<unsigned K>
struct KdTree {
	KdTreeNode<K>* root;

	KdTree (vector<Point<K>*> points) {
		if (points.size() > 0)
			root = new KdTreeNode<K>(points, 0);
		else
			root = NULL;
	}

	unsigned correlated(const Point<K>& p, const double radius) const {
		const double radsq = radius * radius;
		if (root)
			return root->correlated(p, radsq);
		else
			return 0;
	}

	friend std::ostream& operator<< (std::ostream& out, const KdTree<K>& tree) {
		return out << "digraph tree {" << *(tree.root) << '}';
	}
};



















int main () {
	// const int count = 6;
	// Point<2>* points[count];
	vector<Point<2>*> points;
	double coords[2];

	coords[0] = 2;
	coords[1] = 3;
	// points[0] = new Point<2>(coords);
	points.push_back(new Point<2>(coords));

	coords[0] = 5;
	coords[1] = 4;
	// points[1] = new Point<2>(coords);
	points.push_back(new Point<2>(coords));

	coords[0] = 9;
	coords[1] = 6;
	// points[2] = new Point<2>(coords);
	points.push_back(new Point<2>(coords));

	coords[0] = 4;
	coords[1] = 7;
	// points[3] = new Point<2>(coords);
	points.push_back(new Point<2>(coords));

	coords[0] = 8;
	coords[1] = 1;
	// points[4] = new Point<2>(coords);
	points.push_back(new Point<2>(coords));

	coords[0] = 7;
	coords[1] = 2;
	// points[5] = new Point<2>(coords);
	points.push_back(new Point<2>(coords));

	KdTree<2> tree(points);

	using std::cout;
	using std::endl;

	// debug: output points
	// for (unsigned i = 0; i < 6; ++i)
	// 	cout << *points[i] << endl;

	// cout << tree << endl;

	// two point correlation
	// unsigned count = tree.correlated(*points[1], 3.0);
	unsigned count = 0;
	{
		const double radius = 3.0;
		for (unsigned i = 0; i < points.size(); ++i) {
			// cout << endl << i << '\t' << *points[i] << endl;
			// cout << tree.correlated(*points[i], radius) << endl;
			count += tree.correlated(*points[i], radius);
		}
		count = (count - points.size()) / 2;
	}
	cout << count << endl;

	//	CLEANUP
	for (unsigned i = 0; i < 6; ++i)
		delete points[i];

	return 0;
}
