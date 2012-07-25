#ifndef ___KD_TREE_H___
#define ___KD_TREE_H___

// Library includes
#include <Galois/Accumulator.h>

#include "kdtree-node.h"

Galois::GAccumulator<unsigned> count;

template<unsigned K>
struct KdTree {
	struct Correlator {
		const KdTree<K>& tree;
		const double radius;

		Correlator (const KdTree& _tree, const double _radius)
		: tree(_tree)
		, radius(_radius)
		// , count(_count)
		{}

		//	Galois functor
		template<typename Context>
		void operator() (Point<K>** p, Context&) {
			count.get() += tree.correlated(**p, radius);
		}
	};


	/////	Instance

	KdTreeNode<K>* root;

	KdTree (vector<Point<K>*> points) {
		if (points.size() > 0)
			root = new KdTreeNode<K>(points, 0);
		else
			root = NULL;
	}

	unsigned correlated (const vector<Point<K>*>& points, const double radius) const {
		unsigned count = 0;
		for (unsigned i = 0; i < points.size(); ++i)
			count += correlated(*points[i], radius);
		return (count - points.size()) / 2;
	}

	unsigned correlated (const vector<Point<K>*>& points, const double radius, std::ostream& out) const {
		unsigned count = 0;
		double percent = 100.0 / (double) points.size();
		out.setf(std::ios::right | std::ios::fixed |  std::ios::showpoint);
		out.width(3);
		out.precision(2);
		for (unsigned i = 0; i < points.size(); ++i) {
			count += correlated(*points[i], radius);
			out << "Correlating:\t" << i * percent << "%\r";
		}
		out << "\rCorrelating:\t100.00%";
		return (count - points.size()) / 2;
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

#endif//___KD_TREE_H___