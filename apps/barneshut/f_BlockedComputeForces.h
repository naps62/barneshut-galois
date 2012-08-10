#ifndef ___F_BLOCKED_COMPUTE_FORCES_H___
#define ___F_BLOCKED_COMPUTE_FORCES_H___

#include <Galois/Accumulator.h>

#include "Octree.h"

namespace Barneshut {

struct BlockedComputeForces {
	// Optimize runtime for no conflict case
	typedef int tt_does_not_need_aborts;

	// holds a stacked node to process later
	struct Frame {
		double dist_sq;
		Octree* node;
		BodiesPtr& bodies;
		Frame(BodiesPtr& _bodies, Octree* _node, double _dist_sq) : dist_sq(_dist_sq), node(_node), bodies(_bodies) { }

		//~Frame() { delete bodies; }
	};

	OctreeInternal* top;
	double diameter;
	double root_dsq;

	double dthf;
	double epssq;

	Galois::GAccumulator<unsigned> * const tTraversalTotal;
	Completeness* comp;

	std::string papiEventName;
	Galois::GAccumulator<long long int> * const papiValueTotal;

	BlockedComputeForces(OctreeInternal* _top, double _diameter, double itolsq, double _dthf, double _epssq, Galois::GAccumulator<unsigned> * const _tTraversalTotal = NULL, const std::string& _papiEventName = "", Galois::GAccumulator<long long int> * const _papiValueTotal = NULL, Completeness* _comp)
	: top(_top)
	, diameter(_diameter)
	, dthf(_dthf)
	, epssq(_epssq)
	, tTraversalTotal(_tTraversalTotal)
	, papiEventName(_papiEventName)
	, papiValueTotal(_papiValueTotal)
	, comp(_comp)
	{
		root_dsq = diameter * diameter * itolsq;
	}

	/**
	 * Operator
	 */
	template<typename Context>
	void operator()(BodiesPtr** bb, Context&) {
		BodiesPtr& bodies = **bb;
		uint bsize = bodies.size();
		Point * acc = new Point[bsize];

		Galois::StatTimer tTraversal;
		tTraversal.start();

		if (papiEventName.empty()) {
			// backup previous acceleration and initialize new accel to 0
			for(uint j = 0; j < bsize; ++j) {
				Body& body = *bodies[j];

				acc[j] = body.acc;

				for(int i = 0; i < 3; ++i)
					body.acc[i] = 0;
			}

			// compute acceleration for this body
			iterate(bodies, root_dsq);

			// compute new velocity
			for(uint j = 0; j < bsize; ++j) {
				Body& body = *bodies[j];
				for(int i = 0; i < 3; ++i)
					body.vel[i] += (body.acc[i] - acc[j][i]) * dthf;
			}
		} else {
			int event;
			char * name = strdup(papiEventName.c_str());
#ifndef NDEBUG
			assert(PAPI_event_name_to_code(name, &event) == PAPI_OK);
			free(name);

			int eventSet = PAPI_NULL;
			assert(PAPI_create_eventset(&eventSet) == PAPI_OK);
			assert(PAPI_add_event(eventSet, event) == PAPI_OK);

			long long int value;
			assert(PAPI_start(eventSet) == PAPI_OK);
#else
			PAPI_event_name_to_code(name, &event);
			free(name);

			int eventSet = PAPI_NULL;
			PAPI_create_eventset(&eventSet);
			PAPI_add_event(eventSet, event);

			long long int value;
			PAPI_start(eventSet);
#endif

			// backup previous acceleration and initialize new accel to 0
			for(uint j = 0; j < bsize; ++j) {
				Body& body = *bodies[j];

				acc[j] = body.acc;

				for(int i = 0; i < 3; ++i)
					body.acc[i] = 0;
			}

			// compute acceleration for this body
			iterate(bodies, root_dsq);

			// compute new velocity
			for(uint j = 0; j < bsize; ++j) {
				Body& body = *bodies[j];
				for(int i = 0; i < 3; ++i)
					body.vel[i] += (body.acc[i] - acc[j][i]) * dthf;
			}

#ifndef NDEBUG
			assert(PAPI_stop(eventSet, &value) == PAPI_OK);
			papiValueTotal->get() += value;

			assert(PAPI_cleanup_eventset(eventSet) == PAPI_OK);
			assert(PAPI_destroy_eventset(&eventSet) == PAPI_OK);
#else
			PAPI_stop(eventSet, &value);
			papiValueTotal->get() += value;

			PAPI_cleanup_eventset(eventSet);
			PAPI_destroy_eventset(&eventSet);
#endif
		}

		tTraversal.stop();
		tTraversalTotal->get() += tTraversal.get_usec();

		delete[] acc;

		comp->lock.lock();
		std::cout << "\rfinished " << comp->val++ << " / " << comp->total;
		comp->lock.unlock();
	}

	

	void iterate(BodiesPtr& bodies, double root_dsq) {
		// init work stack with top body
		std::stack<Frame> frame_stack;
		frame_stack.push(Frame(bodies, top, root_dsq));

		Point pos_diff;

		while(!frame_stack.empty()) {
			BodiesPtr* new_block = new BodiesPtr();
			Frame f = frame_stack.top();
			frame_stack.pop();

			for(uint i = 0; i < f.bodies.size(); ++i) {
				Body& body = *(f.bodies[i]);

				computePosDiff(body, f.node, pos_diff);
				double dist_sq = pos_diff.dist_sq();

				if (dist_sq >= f.dist_sq || f.node->isLeaf()) {
					// fix this
					if (&body != f.node)
						handleInteraction(body, f.node, dist_sq, pos_diff);
				} else {
					new_block->push_back(&body);
				}
			}

			if (new_block->size() > 0) {
				double dist_sq = f.dist_sq * 0.25;

				for(int i = 0; i < 8; ++i) {
					OctreeInternal* next = static_cast<OctreeInternal*>(f.node);
					Octree* node = next->child[i];
					if (node == NULL)
						break;

					frame_stack.push(Frame(*new_block, node, dist_sq));
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

}

#endif//___F_BLOCKED_COMPUTE_FORCES_H___