#ifndef ___F_CLEAN_COMPUTE_FORCES___
#define ___F_CLEAN_COMPUTE_FORCES___

#include "config.h"
#include "Octree.h"

namespace Barneshut {

struct CleanComputeForces {
	// Optimize runtime for no conflict case
	typedef int tt_does_not_need_aborts;

	// holds a stacked node to process later
	struct Frame {
		double dist_sq;
		OctreeInternal* node;
		Frame(OctreeInternal* _node, double _dist_sq) : dist_sq(_dist_sq), node(_node) { }
	};

	OctreeInternal* top;
	double diameter;
	double root_dsq;

	double dthf;
	double epssq;

	CleanComputeForces(OctreeInternal* _top, double _diameter, double itolsq, double _dthf, double _epssq)
	: top(_top)
	, diameter(_diameter)
	, dthf(_dthf)
	, epssq(_epssq)
	{
		root_dsq = diameter * diameter * itolsq;
	}

	/**
	 * Operator
	 */
	template<typename Context>
	void operator()(Body* bb, Context&) {
		Body& body = *bb;

		// backup previous acceleration and initialize new accel to 0
		Point acc = body.acc;
		for(int i = 0; i < 3; ++i)
			body.acc[i] = 0;

		// compute acceleration for this body
		iterate(body, root_dsq);

		// compute new velocity
		for(int i = 0; i < 3; ++i)
			body.vel[i] += (body.acc[i] - acc[i]) * dthf;
	}


	void iterate(Body& body, double root_dsq) {
		// init work stack with top body
		std::stack<Frame> frame_stack;
		frame_stack.push(Frame(top, root_dsq));

		Point pos_diff;

		while(!frame_stack.empty()) {
			Frame f = frame_stack.top();
			frame_stack.pop();

			computePosDiff(body, f.node, pos_diff);
			double dist_sq = pos_diff.dist_sq();

			if (dist_sq >= f.dist_sq) {
				handleInteraction(body, f.node, dist_sq, pos_diff);
				continue;
			}

			dist_sq = f.dist_sq * 0.25;

			for(int i = 0; i < 8; ++i) {
				Octree* next = f.node->child[i];
				if (next == NULL)
					break;

				if (next->isLeaf()) {
					// Check if it is me
					if (&body != next) {
						computePosDiff(body, next, pos_diff);
						double new_dist_sq = pos_diff.dist_sq();
						handleInteraction(body, next, new_dist_sq, pos_diff);
					}
				} else {
					frame_stack.push(Frame(static_cast<OctreeInternal*>(next), dist_sq));
				}
			}
		}
	}

	private:
		void handleInteraction(Body& body, Octree* node, double dist_sq, Point& pos_diff) {
			dist_sq += epssq;
			double idr = 1 / sqrt(dist_sq);
			double nphi = node->mass * idr;
			double scale = nphi * idr * idr;

			for(int i = 0; i < 3; ++i)
				body.acc[i] += pos_diff[i] * scale;
		}

		void computePosDiff(Body& body, Octree* node, Point& result) {
			for(int i = 0; i < 3; ++i)
				result[i] = node->pos[i] - body.pos[i];
		}
};

}//	namespace Barneshut

#endif//___F_CLEAN_COMPUTE_FORCES___