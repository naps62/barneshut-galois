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

	BlockedComputeForces(OctreeInternal* _top, double _diameter) :
		top(_top),
		diameter(_diameter) {
		root_dsq = diameter * diameter * config.itolsq;
	}


	/**
	 * Operator
	 */
	template<typename Context>
	void operator()(BodiesPtr** bb, Context&) {
		BodiesPtr& bodies = **bb;
		uint bsize = bodies.size();
		std::cout << "here" << bsize << std::endl;
		// backup previous acceleration and initialize new accel to 0
		Point acc[bsize];
		for(uint j = 0; j < bsize; ++j) {
			Body& body = *bodies[j];

			std::cout << "ini " << body.id << " " << body.pos << std::endl;
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
				body.vel[i] += (body.acc[i] - acc[j][i]) * config.dthf;
			//std::cout << "end " << bodies[j].id << " " << bodies[j].vel << std::endl;
		}
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
			std::cout << "using body at " << &body << std::endl;
			dist_sq += config.epssq;
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