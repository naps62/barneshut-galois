#ifndef _COMPLETENESS_H
#define _COMPLETENESS_H


#include <Galois/Runtime/ll/SimpleLock.h>

struct Completeness {
	GaloisRuntime::LL::SimpleLock<true> lock;
	uint val;

	Completeness() { val = 0; }
};

#endif // _COMPLETENESS_H