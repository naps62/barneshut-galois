#ifndef ___POINT_H___
#define ___POINT_H___

#include <iostream>
#include <vector>

#include "ninja.h"

// Library includes
#include <boost/iterator/transform_iterator.hpp>

template<unsigned K>
struct Point {
	double coords[K];

	typedef std::vector<Point<K>*> Points;
	typedef typename Points::iterator PointsIterator;

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

	/////	Static:
	static
	boost::transform_iterator<Deref<Point<K>*>, PointsIterator> 
	wrap(PointsIterator it) {
		return boost::make_transform_iterator(it, Deref<Point<K>*>());
	}

	/*	Blocked stuff
	static
	boost::transform_iterator<Deref<BodiesPtr*>, BodyBlocks::iterator>
	wrap(BodyBlocks::iterator it) {
		return boost::make_transform_iterator(it, Deref<BodiesPtr*>());
	}
	*/



	/////	Instance:

	Point () {
		memset(coords, 0, sizeof(double) * K);
	}

	Point (double _coords[K]) {
		for (unsigned i = 0; i < K; ++i)
			coords[i] = _coords[i];
	}

	Point (const double v) {
		for (unsigned i = 0; i < K; ++i)
			coords[i] = v;
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
	double lengthRaised () const {
		double lsq = 0.0;
		for (unsigned i = 0; i < K; ++i)
			lsq += coords[i] * coords[i];
		return lsq;
	}

	double distanceRaised (const Point& p) const {
		Point q(*this);
		q -= p;
		return q.lengthRaised();
	}


	///// Setters
	Point<K>& set(unsigned index, double value) { coords[index] = value; return *this; }

	Point<K>& setMinimum (const Point<K>& p) {
		for (unsigned i = 0; i < K; ++i)
			coords[i] = fmin(coords[i], p[i]);
		return *this;
	}

	Point<K>& setMaximum (const Point<K>& p) {
		for (unsigned i = 0; i < K; ++i)
			coords[i] = fmax(coords[i], p[i]);
		return *this;
	}


	/** Limits the point's coordinates to values limited by two other points.
	 */
	Point<K>& limit (const Point<K>& min, const Point<K>& max) {
		for (unsigned i = 0; i < K; ++i) {
			// double vmax = fmax(min[i], max[i]);
			// double vmin = fmin(min[i], max[i]);
			coords[i] = fmax(coords[i], min[i]);
			coords[i] = fmin(coords[i], max[i]);
		}
	}


	///// Operators
	Point<K>& operator-= (const Point<K>& p) {
		for (unsigned i = 0; i < K; ++i)
			coords[i] -= p.coords[i];
		return *this;
	}
	Point<K>& operator*= (const double v) {
		for (unsigned i = 0; i < K; ++i)
			coords[i] *= v;
		return *this;
	}

	Point<K> operator* (const double v) const {
		Point<K> r(*this);
		r *= v;
		return r;
	}


	double operator[] (const unsigned index) const { return coords[index]; }
	double& operator[] (const unsigned index) { return coords[index]; }


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

#endif//___POINT_H___