struct BuildOctree {
  // NB: only correct when run sequentially
  typedef int tt_does_not_need_stats;

  OctreeInternal* root;
  double root_radius;

  BuildOctree(OctreeInternal* _root, double radius) :
    root(_root),
    root_radius(radius) { }

  template<typename Context>
  void operator()(Body* b, Context&) {
    insert(b, root, root_radius);
  }

  void insert(Body* b, OctreeInternal* node, double radius) {
    int index = getIndex(node->pos, b->pos);

    assert(!node->isLeaf());

    Octree *child = node->child[index];
    
    if (child == NULL) {
      node->child[index] = b;
      return;
    }
    
    radius *= 0.5;
    if (child->isLeaf()) {
      // Expand leaf
      Body* n = static_cast<Body*>(child);
      Point new_pos(node->pos);
      updateCenter(new_pos, index, radius);
      OctreeInternal* new_node = new OctreeInternal(new_pos);

      assert(n->pos != b->pos);
      
      insert(b, new_node, radius);
      insert(n, new_node, radius);
      node->child[index] = new_node;
    } else {
      OctreeInternal* n = static_cast<OctreeInternal*>(child);
      insert(b, n, radius);
    }
  }
};