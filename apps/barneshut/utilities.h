#ifndef ___UTILITIES_H___
#define ___UTILITIES_H___

#include <Galois/Runtime/ll/TID.h>

namespace Barneshut {
	typedef std::vector<Body>   Bodies;
	typedef std::vector<Body*>  BodiesPtr;
	typedef std::vector<BodiesPtr*> BodyBlocks;

	inline
	unsigned long getTID() { return GaloisRuntime::LL::getTID(); }

	inline
	double nextDouble() { return rand() / (double) RAND_MAX; }

	/**
	 * Bitwise stuff
	 * index initial: 000 (em binário)
	 * a.x < b.x:     001
	 * a.y < b.y:     010
	 * a.z < b.z:     100
	 *
	 * For each axis, one of the bits is set to 1 if a < b in that axis
	 * Why? still dont know
	 */
	inline int getIndex(const Point& a, const Point& b) {
		int index = 0;
		if (a.x < b.x)
			index += 1;
		if (a.y < b.y)
			index += 2;
		if (a.z < b.z)
			index += 4;
		return index;
	}



	/**
	 * Used to move a point by a given space in each axis
	 * Index comes from the getIndex function up there.
	 * 
	 * index is used to know in which direction to update every axis
	 */
	inline void updateCenter(Point& p, int index, double radius) {
		for (int i = 0; i < 3; i++) {
			double v = (index & (1 << i)) > 0 ? radius : -radius;
			p[i] += v;
		}
	}



	/**
	 * Generates random input according to the Plummer model, which is more
	 * realistic but perhaps not so much so according to astrophysicists
	 */
	void generateInput(Bodies& bodies, int nbodies, int seed) {
		double v, sq, scale;
		Point p;
		double PI = boost::math::constants::pi<double>();

		srand(seed);

		double rsc = (3 * PI) / 16;
		double vsc = sqrt(1.0 / rsc);

		int nextId = 0;

		for (int body = 0; body < nbodies; body++) {
			double r = 1.0 / sqrt(pow(nextDouble() * 0.999, -2.0 / 3.0) - 1);
			do {
				for (int i = 0; i < 3; i++)
					p[i] = nextDouble() * 2.0 - 1.0;
				sq = p.x * p.x + p.y * p.y + p.z * p.z;
			} while (sq > 1.0);
			scale = rsc * r / sqrt(sq);

			Body b;
			b.mass = 1.0 / nbodies;
			for (int i = 0; i < 3; i++)
				b.pos[i] = p[i] * scale;

			do {
				p.x = nextDouble();
				p.y = nextDouble() * 0.1;
			} while (p.y > p.x * p.x * pow(1 - p.x * p.x, 3.5));
			v = p.x * sqrt(2.0 / sqrt(1 + r * r));
			do {
				for (int i = 0; i < 3; i++)
					p[i] = nextDouble() * 2.0 - 1.0;
				sq = p.x * p.x + p.y * p.y + p.z * p.z;
			} while (sq > 1.0);
			scale = vsc * v / sqrt(sq);
			for (int i = 0; i < 3; i++)
				b.vel[i] = p[i] * scale;

			bodies.push_back(b);
			b.id = nextId;
			nextId++;
		}
	}
}

#endif//___UTILITIES_H___
