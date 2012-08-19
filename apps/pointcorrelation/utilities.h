#ifndef ___UTILITIES_H___
#define ___UTILITIES_H___

// C++ includes
// Library includes
#include <boost/math/constants/constants.hpp>
#include <Galois/Runtime/ll/TID.h>

#include "point.h"

inline
unsigned long getTID() { return GaloisRuntime::LL::getTID(); }

inline
double randomDouble () { return rand() / (double) RAND_MAX; }


#define DIM 3
/**
 * Generates random input according to the Plummer model, which is more
 * realistic but perhaps not so much so according to astrophysicists
 * \param n Number of points to generate.
 */
void generateInput(vector<Point<DIM>*>& points, unsigned n, unsigned seed) {
	// double v;
	double sq, scale;
	Point<DIM> p;
	double PI = boost::math::constants::pi<double>();

	srand(seed);

	double rsc = (3 * PI) / 16;
	// double vsc = sqrt(1.0 / rsc);

	// int nextId = 0;

	for (unsigned i = 0; i < n; ++i) {
		double r = 1.0 / sqrt(pow(randomDouble() * 0.999, -2.0 / 3.0) - 1);
		do {
			sq = 0.0;
			for (unsigned i = 0; i < DIM; i++) {
				p[i] = randomDouble() * 2.0 - 1.0;
				sq += p[i] * p[i];
			}
		} while (sq > 1.0);
		scale = rsc * r / sqrt(sq);

		Point<DIM>* q = new Point<DIM>(p * scale);
		// Body b;
		// b.mass = 1.0 / points.size();
		// for (int i = 0; i < DIM; i++)
		// 	b.pos[i] = p[i] * scale;


		// do {
		// 	p[0] = randomDouble();
		// 	p[1] = randomDouble() * 0.1;
		// } while (p[1] > p[0] * p[0] * pow(1 - p[0] * p[0], 3.5));
		// v = p[0] * sqrt(2.0 / sqrt(1 + r * r));

		// do {
		// 	sq = 0.0;
		// 	for (int i = 0; i < DIM; i++) {
		// 		p[i] = randomDouble() * 2.0 - 1.0;
		// 		sq += p[i] * p[i];
		// 	}
		// } while (sq > 1.0);
		// scale = vsc * v / sqrt(sq);

		// for (int i = 0; i < DIM; i++)
		// 	b.vel[i] = p[i] * scale;

		points.push_back(q);
		// points.push_back(b);
		// b.id = nextId;
		// nextId++;
	}
}

#endif//___UTILITIES_H___
