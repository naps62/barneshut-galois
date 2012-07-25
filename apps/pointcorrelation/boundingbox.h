#ifndef ___BOUNDING_BOX_H___
#define ___BOUNDING_BOX_H___

// C++ includes
#include <iostream>
#include <limits>

#include "point.h"

template<unsigned K>
struct BoundingBox {
	Point<K> min;
	Point<K> max;

	/**
	 * Inits the box with a single point, equal to both min and max
	 */
	explicit BoundingBox(const Point<K>& p) : min(p), max(p) { }

	/**
	 * If not args, box is infinite
	 */
	BoundingBox()
	: min(std::numeric_limits<double>::max())
	, max(std::numeric_limits<double>::min())
	{
		// std::cerr << "Hello from default constructor @ BoundingBox" << std::endl;
		// std::cerr << *this << std::endl;
	}

	BoundingBox (double vmin, double vmax)
	: min(vmin)
	, max(vmax)
	{}

	/**
	 * Given another box, merges both, by giving the smallest boundingBox that wraps both of them
	 * 
	 * eg:
	 * b1 =           ((0,0,0),   (1,1,1))
	 * b2 =           ((2,2,2),   (3,3,3))
	 *
	 * b1.merge(b2) = ((0,0,0),   (3,3,3))
	 */
	void merge(const BoundingBox& bbox) {
		min.setMinimum(bbox.min);
		max.setMaximum(bbox.max);
	}

	/**
	 * Same thing as above, but merges the box with a single point
	 * Basically expands the box up to the fiven point
	 */
	void merge(const Point<K>& p) {
		min.setMinimum(p);
		max.setMaximum(p);
	}

	/** Defines the diameter of the box as the minimum of the diferences of all the coordinates
	 */
	double diameter() const { return (max - min).min(); }

	/** Defines the radius of the box as half the diameter.
	 */
	double radius() const { return diameter() / 2; }

	/** Computes the centroid of the box.
	 */
	Point<K> center() const { return (min + max) * 0.5; }


	/**
	 */
	bool isIn(const Point<K>& p) const {
		bool in = true;
		for (unsigned i = 0; in && i < K; ++i)
			in = in && p[i] >= min[i] && p[i] <= max[i];
		return in;
	}

	/**
	 */
	bool isOut(const Point<K>& p) const { return !isIn(p); }


	/** Computes the point in the box which is closest to the point of reference.
	 */
	Point<K> closest(const Point<K>& p) const {
		Point<K> q;
		for (unsigned i = 0; i < K; ++i) {
			if (p[i] > max[i])
				q.set(i, p[i] - max[i]);
			else if (p[i] < min[i])
				q.set(i, min[i] - p[i]);
		}
		return q;
	}


	/** Calculates the minimum distance from a point to this box.
	 * \param p Point of reference.
	 */
	double minimumDistanceRaised (const Point<K>& p) const { return closest(p).lengthRaised(); }


	//// Operators
	friend
	std::ostream& operator<<(std::ostream& os, const BoundingBox& b) {
		return os << "(min:" << b.min << " max:" << b.max << ")";
	}
};

#endif//___BOUNDING_BOX_H___
