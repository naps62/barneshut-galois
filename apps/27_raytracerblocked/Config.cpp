#include "Config.h"

using llvm::cl::desc;
using llvm::cl::init;

Config::Config()
:	w       ("w",    desc("Output image width"),  init(  200)),
	h       ("h",    desc("Output image height"), init(  200)),
	spp     ("spp",  desc("Samples per pixel"),   init(  256)),
	maxdepth("d",    desc("Max ray depth"),       init(    6)),
	n       ("n",    desc("Number of spheres"),   init(    2)),
	block   ("b",    desc("Block size"),          init(    1)),
	dump    ("dump", desc("Dump BVH Tree"),       init(false)),
	outfile ("out",  desc("Output file"),         init(std::string("image.ppm"))),
	papi    ("papi", desc("Activate PAPI"),       init(false))
{ }