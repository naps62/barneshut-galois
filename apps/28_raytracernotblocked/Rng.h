#ifndef _RNG_H
#define _RNG_H
#include <stdlib.h>
#include <limits>
#include <iostream>
#include <stdint.h>
using namespace std;
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/uniform_real.hpp>

struct RNG {
	typedef boost::mt19937 RNGType;

	RNGType rng;
	const boost::uniform_real<> range;
	boost::variate_generator<RNGType, boost::uniform_real<> > dice;

	RNG()
	:	range(0.0, 1.0),
		dice(rng, range) { }

	void init(time_t seed) {
		rng = RNGType(seed);
		dice = boost::variate_generator<RNGType, boost::uniform_real<> >(rng, range);
	}

	double operator() () {
		return dice();
	}
};

#endif // _RNG_H