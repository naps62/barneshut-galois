#include "UID.h"

unsigned int UID::nextID;

unsigned int UID::get() {
	return nextID++;
}