#ifndef ___F_ADVANCE_BODIES_H___
#define ___F_ADVANCE_BODIES_H___

#include "Octree.h"

namespace Barneshut {

	/**
	 * Computes the position of a body after a time step.
	 */
	struct AdvanceBodies {
		// Optimize runtime for no conflict case
		typedef int tt_does_not_need_aborts;

		double dthf;
		double dtime;

		AdvanceBodies(double _dthf, double _dtime)
		: dthf(_dthf)
		, dtime(_dtime)
		{ }

		template<typename Context>
			void operator()(Body* bb, Context&) {
				Body& b = *bb;
				Point dvel(b.acc);
				dvel *= dthf;

				Point velh(b.vel);
				velh += dvel;

				for (int i = 0; i < 3; ++i)
					b.pos[i] += velh[i] * dtime;
				for (int i = 0; i < 3; ++i)
					b.vel[i] = velh[i] + dvel[i];
			}
	};

}

#endif//___F_ADVANCE_BODIES_H___
