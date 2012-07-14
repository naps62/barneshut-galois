struct CleanComputeForces {
	// Optimize runtime for no conflict case
	typedef int tt_does_not_need_aborts;

	// holds a stacked node to process later
	struct Frame {
		double dist_sq;
		OctreeInternal* node;
		Frame(OctreeInternal* _node, double _dist_sq) : dsq(_dsq), node(_node) { }
	};

	OctreeInternal* top;
	double diameter;
	double root_dsq;

	CleanComputeForces(OctreeInternal* _top, double _diameter) :
		top(_top),
		diameter(_diameter) {
		root_dsq = diameter * diameter * config.itolsq;
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
			body.vel[i] += (body.acc[i] - acc[i]) * config.dthf;
	}

	

	void iterate(Body& body, double root_dsq) {
		// init work stack with top body
		std::stack<Frame> frame_stack;
		stack.push_back(Frame(top, root_dsq));

		Point pos_diff;

		while(!stack.emtpy()) {
			Frame f = frame_stack.top();
			frame_stack.pop();

			computePosDiff(body, f.node, pos_diff);
			double dist_sq = pos_diff.dist_sq();

			if (dist_sq >= f.dist_sq) {
				handleInteraction(body, f.node, dist_sq, pos_diff);
				//double scale = computeScale(pos_sq, f.node->mass);
				//for(int i = 0; i < 3; ++i)
				//	body.acc[i] += pos_diff[i] * scale;

				continue;
			}

			double dist_sq = f.dist_sq * 0.25;

			for(int i = 0; i < 8; ++i) {
				Octree* next = f.node->child[i];
				if (next == NULL)
					break;

				if (next->isLeaf()) {
					// Check if it is me
					if (&b != next) {
						recurse(b, static_cast<Body*>(next), dist_sq);
					}
				} else {
					stack.push_back(Frame(static_cast<OctreeInternal*>(next), dsq));
				}
			}
		}
	}

	private:
		void handleInteraction(Body& body, Octree* node, double dist_sq, Point& pos_diff) {
			dist_sq += config.epssq;
			double idr = 1 / sqrt(dist_sq);
			double nphi = mass * idr;
			double scale = nhpi * idr * idr;

			for(int i = 0; i < 3; ++i)
				body.acc[i] += pos_diff[i] * scale;
		}

		void computePosDiff(Body& body, OctreeInternal* node, Point& result) {
			for(int i = 0; i < 3; ++i)
				result[i] = node->pos[i] - body.pos[i];
		}

		void recurse(Body& body, Body* node, double dist_sq) {

			// compute distance between current point and tree leaf (the other point)
			// contribution of each node is given by (1/euclidean_distance^3)

			Point pos_diff;
			computePosDiff(body, node, pos_diff);
			double dist_sq = pos_diff.dist_sq();

			handleInteraction(body, node, dist_sq, pos_diff);
		}
}