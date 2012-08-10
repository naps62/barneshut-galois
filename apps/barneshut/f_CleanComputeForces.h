#ifndef ___F_CLEAN_COMPUTE_FORCES___
#define ___F_CLEAN_COMPUTE_FORCES___

//	Libraries includes
#include <Galois/Accumulator.h>

//	local includes
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

	Galois::GAccumulator<unsigned> * const tTraversalTotal;
	Completeness* comp;

	std::string papiEventName;
	Galois::GAccumulator<long long int> * const papiValueTotal;

	CleanComputeForces(OctreeInternal* _top, double _diameter, double itolsq, double _dthf, double _epssq, Galois::GAccumulator<unsigned> * const _tTraversalTotal = NULL, const std::string& _papiEventName = "", Galois::GAccumulator<long long int> * const _papiValueTotal = NULL, Completeness* _comp = NULL)
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
	void operator()(Body* bb, Context&) {
		Body& body = *bb;
		Galois::StatTimer tTraversal;
		tTraversal.start();

		if (papiEventName.empty()) {
			// backup previous acceleration and initialize new accel to 0
			Point acc = body.acc;
			for(int i = 0; i < 3; ++i)
				body.acc[i] = 0;

			// compute acceleration for this body
			iterate(body, root_dsq);

			// compute new velocity
			for(int i = 0; i < 3; ++i)
				body.vel[i] += (body.acc[i] - acc[i]) * dthf;
		} else {
#ifndef NDEBUG
			int event;
			char * name = strdup(papiEventName.c_str());
			assert(PAPI_event_name_to_code(name, &event) == PAPI_OK);
			free(name);

			int eventSet = PAPI_NULL;
			assert(PAPI_create_eventset(&eventSet) == PAPI_OK);
			assert(PAPI_add_event(eventSet, event) == PAPI_OK);

			long long int value;
			assert(PAPI_start(eventSet) == PAPI_OK);
#else
			int event;
			char * name = strdup(papiEventName.c_str());
			int result = PAPI_event_name_to_code(name, &event);
			// if (result != PAPI_OK)
			// 	std::cerr << "PAPI_event_name_to_code: " << PAPI_strerror(result) << std::endl;
			free(name);

			int eventSet = PAPI_NULL;
			result = PAPI_create_eventset(&eventSet);
			// if (result != PAPI_OK)
			// 	std::cerr << "PAPI_create_eventset: " << PAPI_strerror(result) << std::endl;
			result = PAPI_add_event(eventSet, event);
			// if (result != PAPI_OK)
			// 	std::cerr << "PAPI_add_event: " << PAPI_strerror(result) << std::endl;


			long long int value;
			result = PAPI_start(eventSet);
			// if (result != PAPI_OK)
			// 	std::cerr << "PAPI_start: " << PAPI_strerror(result) << std::endl;
#endif

			// backup previous acceleration and initialize new accel to 0
			Point acc = body.acc;
			for(int i = 0; i < 3; ++i)
				body.acc[i] = 0;

			// compute acceleration for this body
			iterate(body, root_dsq);

			// compute new velocity
			for(int i = 0; i < 3; ++i)
				body.vel[i] += (body.acc[i] - acc[i]) * dthf;

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

		comp->lock.lock();
		std::cout << "\finished " << comp->val++ << " / " << comp->total << endl;
		comp->lock.unlock();
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