#include "Config.h"

using llvm::cl::desc;
using llvm::cl::init;

Config::Config()
:	w       ("w",    desc("Output image width"),  init( 1)),
	h       ("h",    desc("Output image height"), init(  1)),
	spp     ("spp",  desc("Samples per pixel"),   init(    1)),
	maxdepth("d",    desc("Max ray depth"),       init(    1)),
	n       ("n",    desc("Number of spheres"),   init(    0)),
	block   ("b",    desc("Block size"),          init(    1)),
	dump    ("dump", desc("Dump BVH Tree"),       init(false)),
	outfile ("out",  desc("Output file"),         init(std::string("image.ppm")))
{ }