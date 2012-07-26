/**
 * Functor
 *
 * Computes the force interacting with each body
 */
struct ComputeForces {
  // Optimize runtime for no conflict case
  typedef int tt_does_not_need_aborts;

  OctreeInternal* top;
  double diameter;
  double root_dsq;

  ComputeForces(OctreeInternal* _top, double _diameter) :
    top(_top),
    diameter(_diameter) {
    root_dsq = diameter * diameter * config.itolsq;
  }
  
  /**
   * Main operator
   * Computes the force affecting each body
   */
  template<typename Context>
  void operator()(Body* bb, Context&) {
    Body& b = *bb;

    // backup previous acceleration, and initialize new accel to 0
    Point p = b.acc;
    for (int i = 0; i < 3; i++)
      b.acc[i] = 0;

    // compute acceleration for body b
    //recurse(b, top, root_dsq);
    iterate(b, root_dsq);

    // compute new velocity based on (new_acceleration - previous_acceleration)
    for (int i = 0; i < 3; i++)
      b.vel[i] += (b.acc[i] - p[i]) * config.dthf;
  }

  /**
   * Recursive step for a Body
   *   Recursion ends here
   */
  void recurse(Body& b, Body* node, double dsq) {

    // compute distance between current point, and tree leaf (the other point)
    // contribution of each node is given by 1/(euclidean_distance^3)

    Point p;
    for (int i = 0; i < 3; i++)
      p[i] = node->pos[i] - b.pos[i];

    double psq = p.x * p.x + p.y * p.y + p.z * p.z;
    psq += config.epssq;
    double idr = 1 / sqrt(psq);
    // b.mass is fine because every body has the same mass
    double nphi = b.mass * idr;
    double scale = nphi * idr * idr;
    for (int i = 0; i < 3; i++) 
      b.acc[i] += p[i] * scale;
  }

  struct Frame {
    double dsq;
    OctreeInternal* node;
    Frame(OctreeInternal* _node, double _dsq) : dsq(_dsq), node(_node) { }
  };

  /**
   * Iterates all bodies in work queue, and computes accel for each one
   */
  void iterate(Body& b, double root_dsq) {
    std::vector<Frame> stack;
    stack.push_back(Frame(top, root_dsq));

    Point p;
    while (!stack.empty()) {
      Frame f = stack.back();
      stack.pop_back();

      // compute distance of that node to the point
      for (int i = 0; i < 3; i++)
        p[i] = f.node->pos[i] - b.pos[i];

      double psq = p.x * p.x + p.y * p.y + p.z * p.z;
      if (psq >= f.dsq) {
        // Node is far enough away, summarize contribution
        psq += config.epssq;
        double idr = 1 / sqrt(psq);
        double nphi = f.node->mass * idr;
        double scale = nphi * idr * idr;
        for (int i = 0; i < 3; i++) 
          b.acc[i] += p[i] * scale;
        
        continue;
      }

      double dsq = f.dsq * 0.25;
      
      for (int i = 0; i < 8; i++) {
        Octree *next = f.node->child[i];
        if (next == NULL)
          break;
        if (next->isLeaf()) {
          // Check if it is me
          if (&b != next) {
            recurse(b, static_cast<Body*>(next), dsq);
          }
        } else {
          stack.push_back(Frame(static_cast<OctreeInternal*>(next), dsq));
        }
      }
    }
  }

  /*void recurse(Body& b, OctreeInternal* node, double dsq) {
    Point p;

    for (int i = 0; i < 3; i++)
      p[i] = node->pos[i] - b.pos[i];
    double psq = p.x * p.x + p.y * p.y + p.z * p.z;
    if (psq >= dsq) {
      // Node is far enough away, summarize contribution
      psq += config.epssq;
      double idr = 1 / sqrt(psq);
      double nphi = node->mass * idr;
      double scale = nphi * idr * idr;
      for (int i = 0; i < 3; i++) 
        b.acc[i] += p[i] * scale;
      
      return;
    }

    dsq *= 0.25;
    
    for (int i = 0; i < 8; i++) {
      Octree *next = node->child[i];
      if (next == NULL)
        break;
      if (next->isLeaf()) {
        // Check if it is me
        if (&b != next) {
          recurse(b, static_cast<Body*>(next), dsq);
        }
      } else {
        recurse(b, static_cast<OctreeInternal*>(next), dsq);
      }
    }
  }*/
};