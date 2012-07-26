// smallpt, a Path Tracer by Kevin Beason, 2008 
// Make : g++ -O3 -fopenmp smallpt.cpp -o smallpt 
//        Remove "-fopenmp" for g++ version < 4.2 

#include <cmath>
#include <stdlib.h>
#include <vector>

#include "Galois/Galois.h"
#include "Galois/Statistic.h"
#include "Lonestar/BoilerPlate.h"


/** clamps a value between 0 and 1 */
inline double clamp(double x) {
	if (x < 0)      return 0;
	else if (x > 1) return 1;
	else            return x;
}

/** interpolates a double to a 0-255 integer scale */
inline int toInt(double x) {
	return int(pow(clamp(x), 1 / 2.2) * 255 + 0.5);
} 


#include "structs/All.h"
#include "Config.h"
#include "f_ClampImage.h"
#include "f_RayTrace.h"

Config config;

typedef std::vector<Ray> Rays;
typedef std::vector<Object*> ObjectList;

#include "scene.h"


/***********************************************************
 * MAIN
 **********************************************************/
int main(int argc, char *argv[]) {
	Galois::StatManager M;
	LonestarStart(argc, argv, "Ray Tracing");

	std::cerr << std::fixed;
	std::cerr.width(2);
	std::cerr.precision(2);
	//std::cerr << std::endl;

	/*std::cerr	<< "configuration: "
					<< config.w << "x" << config.h
					<< " (" << config.spp << " spp)"
					<< std::endl << std::endl;

	std::cout << "Num. of threads: " << numThreads << std::endl;*/
	

	Galois::StatTimer T("Total");
	T.start();
	Scene scene(config);
	scene.raytrace();
	scene.save();
	T.stop();

	std::cerr << std::endl << std::endl;
}