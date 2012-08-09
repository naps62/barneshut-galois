#ifndef ___KD_TREE_H___
#define ___KD_TREE_H___

// C++ includes
#include <vector>
using std::vector;

// Library includes
#include <Galois/Accumulator.h>

#include "kdtree-node.h"

Galois::GAccumulator<unsigned> count;

template<unsigned K>
struct KdTree {
	struct Correlator {
		const KdTree<K>& tree;
		const double radius;

		//	for average time per point
		Galois::GAccumulator<unsigned long> * const tTraversalTotal;

		//	PAPI data
		std::string papiEventName;
		Galois::GAccumulator<long long int> * const papiValueTotal;

		Correlator (const KdTree& _tree, const double _radius, Galois::GAccumulator<unsigned long> * const _tTraversalTotal, const std::string& _papiEventName = "", Galois::GAccumulator<long long int> * const _papiValueTotal = NULL)
		: tree(_tree)
		, radius(_radius)
		, tTraversalTotal(_tTraversalTotal)
		, papiEventName(_papiEventName)
		, papiValueTotal(_papiValueTotal)
		{}

		//	Galois functor
		template<typename Context>
		void operator() (Point<K>** p, Context&) {
			Galois::StatTimer tTraversal;
			if (!papiEventName.empty()) {
#ifndef NDEBUG
				int eventSet = PAPI_NULL;
				assert(PAPI_create_eventset(&eventSet) == PAPI_OK);

				int event;
				char * name = strdup(papiEventName.c_str());
				assert(PAPI_event_name_to_code(name, &event) == PAPI_OK);
				free(name);

				assert(PAPI_add_event(eventSet, event) == PAPI_OK);

				long long int value;

				assert(PAPI_start(eventSet) == PAPI_OK);
#else
				int eventSet = PAPI_NULL;
				PAPI_create_eventset(&eventSet);
				int event;
				char * name = strdup(papiEventName.c_str());
				PAPI_event_name_to_code(name, &event);
				free(name);
				PAPI_add_event(eventSet, event);
				long long int value;
				PAPI_start(eventSet);
#endif
				
				tTraversal.start();
				count.get() += tree.correlated(**p, radius);
				tTraversal.stop();
				tTraversalTotal->get() += tTraversal.get_usec();

#ifndef NDEBUG
				assert(PAPI_stop(eventSet, &value) == PAPI_OK);

				//	gather values
				papiValueTotal->get() += value;

				assert(PAPI_cleanup_eventset(eventSet) == PAPI_OK);
				assert(PAPI_destroy_eventset(&eventSet) == PAPI_OK);
#else
				PAPI_stop(eventSet, &value);
				papiValueTotal->get() += value;
				PAPI_cleanup_eventset(eventSet);
				PAPI_destroy_eventset(&eventSet);
#endif
			} else {
				tTraversal.start();
				count.get() += tree.correlated(**p, radius);
				tTraversal.stop();
				tTraversalTotal->get() += tTraversal.get_usec();
			}
		}
	};

	struct BlockedCorrelator {
		const KdTree<K>& tree;
		const double radius;

		//	for average time per point
		Galois::GAccumulator<unsigned long> * const tTraversalTotal;

		//	PAPI data
		std::string papiEventName;
		Galois::GAccumulator<long long int> * const papiValueTotal;

		BlockedCorrelator (const KdTree& _tree, const double _radius, Galois::GAccumulator<unsigned long> * const _tTraversalTotal, const std::string& _papiEventName = "", Galois::GAccumulator<long long int> * const _papiValueTotal = NULL)
		: tree(_tree)
		, radius(_radius)
		, tTraversalTotal(_tTraversalTotal)
		, papiEventName(_papiEventName)
		, papiValueTotal(_papiValueTotal)
		{}

		//	Galois functor
		template<typename Context>
		void operator() (vector<Point<K>*>* b, Context&) {
			Galois::StatTimer tTraversal;
			if (!papiEventName.empty()) {
				int eventSet = PAPI_NULL;
				assert(PAPI_create_eventset(&eventSet) == PAPI_OK);

				int event;
				char * name = strdup(papiEventName.c_str());
				assert(PAPI_event_name_to_code(name, &event) == PAPI_OK);
				free(name);

				assert(PAPI_add_event(eventSet, event) == PAPI_OK);

				long long int value;
				assert(PAPI_start(eventSet) == PAPI_OK);

				tTraversal.start();
				count.get() += tree.correlated(*b, radius);
				tTraversal.stop();
				tTraversalTotal->get() += tTraversal.get_usec();

				assert(PAPI_stop(eventSet, &value) == PAPI_OK);

				//	gather values
				papiValueTotal->get() += value;

				assert(PAPI_cleanup_eventset(eventSet) == PAPI_OK);
				assert(PAPI_destroy_eventset(&eventSet) == PAPI_OK);
			} else {
				tTraversal.start();
				count.get() += tree.correlated(*b, radius);
				tTraversal.stop();
				tTraversalTotal->get() += tTraversal.get_usec();
			}
		}
	};


	/////	Instance

	KdTreeNode<K>* root;

	KdTree (vector<Point<K>*> points) {
		if (points.size() > 0)
			root = new KdTreeNode<K>(points, 0);
		else
			root = NULL;
	}

	unsigned correlated (const typename Point<K>::Block& b, const double radius) const {
		const double radrsd = pow(radius, K);
		if (root)
			return root->correlated(b, radrsd);
		else
			return 0;
	}

	// unsigned correlated (const vector<Point<K>*>& points, const double radius) const {
	// 	unsigned count = 0;
	// 	for (unsigned i = 0; i < points.size(); ++i)
	// 		count += correlated(*points[i], radius);
	// 	return (count - points.size()) / 2;
	// }

	unsigned correlated (const vector<Point<K>*>& points, const double radius, std::ostream& out) const {
		unsigned count = 0;
		double percent = 100.0 / (double) points.size();
		out.setf(std::ios::right | std::ios::fixed |  std::ios::showpoint);
		out.width(3);
		out.precision(2);
		for (unsigned i = 0; i < points.size(); ++i) {
			count += correlated(*points[i], radius);
			out << "Correlating:\t" << i * percent << "%\r";
		}
		out << "\rCorrelating:\t100.00%";
		return (count - points.size()) / 2;
	}

	unsigned correlated(const Point<K>& p, const double radius) const {
		const double radrsd = pow(radius, K);
		if (root)
			return root->correlated(p, radrsd);
		else
			return 0;
	}

	friend std::ostream& operator<< (std::ostream& out, const KdTree<K>& tree) {
		return out << "digraph tree {" << *(tree.root) << '}';
	}
};

#endif//___KD_TREE_H___