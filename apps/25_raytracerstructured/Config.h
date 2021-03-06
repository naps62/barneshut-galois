#ifndef _CONFIG_H
#define _CONFIG_H

#include <string>
#include "llvm/Support/CommandLine.h"
using llvm::cl::opt;

struct Config {

	opt<uint>   w;
	opt<uint>   h;
	opt<uint>   spp;
	opt<uint>   maxdepth;
	opt<uint>   n;
	opt<uint>   dump;
	opt<std::string> outfile;

	Config();
};

#endif // _CONFIG_H