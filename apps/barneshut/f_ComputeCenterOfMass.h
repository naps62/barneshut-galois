#ifndef ___F_COMPUTE_CENTER_OF_MASS_H___
#define ___F_COMPUTE_CENTER_OF_MASS_H___

#include "Octree.h"

namespace Barneshut {

/**
 * Functor
 *
 * Computes the center of mass for each node of the Octree
 */

struct ComputeCenterOfMass {
  // NB: only correct when run sequentially or tree-like reduction
  typedef int tt_does_not_need_stats;
  OctreeInternal* root;

  ComputeCenterOfMass(OctreeInternal* _root) : root(_root) { }

  /**
   * Center of mass for top of the tree is given after recursing the entire tree, computing each node's center of mass
   */
  void operator()() {
    root->mass = recurse(root);
  }

private:
  double recurse(OctreeInternal* node) {
    double mass = 0.0;
    int index = 0;
    Point accum;
    
    // iterate all existing childs of current node
    for (int i = 0; i < 8; i++) {
      Octree* child = node->child[i];
      if (child == NULL)
        continue;

      // Reorganize leaves to be denser up front 
      if (index != i) {
        node->child[index] = child;
        node->child[i] = NULL;
      }
      index++;
      
      // for the leaves, center of mass is direct
      // for inner nodes, it is recursively given
      double m;
      const Point* p;
      if (child->isLeaf()) {
        Body* n = static_cast<Body*>(child);
        m = n->mass;
        p = &n->pos;
      } else {
        OctreeInternal* n = static_cast<OctreeInternal*>(child);
        m = recurse(n);
        p = &n->pos;
      }

      mass += m;
      for (int j = 0; j < 3; j++) 
        accum[j] += (*p)[j] * m;
    }

    node->mass = mass;
    
    if (mass > 0.0) {
      double inv_mass = 1.0 / mass;
      for (int j = 0; j < 3; j++)
        node->pos[j] = accum[j] * inv_mass;
    }

    return mass;
  }
};

}

#endif//___F_COMPUTE_CENTER_OF_MASS_H___