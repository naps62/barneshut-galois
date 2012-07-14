/** Builds the Octree using recursive calls.
 * When it finds a leaf, it is replaced by an internal, and both the leaf and the new body are recursively inserted in the new internal.
 */
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
    // calc the child node where this body should belong to
    int index = getIndex(node->pos, b->pos);

    assert(!node->isLeaf());

    Octree *child = node->child[index];
    
    // if there is no child, make it a leaf node
    if (child == NULL) {
      node->child[index] = b;
      return;
    }
    
    // if child is a leaf, expand it into an OctreeInternal
    radius *= 0.5;
    if (child->isLeaf()) {
      // Expand leaf
      Body* n = static_cast<Body*>(child);
      Point new_pos(node->pos);
      updateCenter(new_pos, index, radius);
      OctreeInternal* new_node = new OctreeInternal(new_pos);

      assert(n->pos != b->pos);
      
      // insert both the old leaf and the new body into the new node
      insert(b, new_node, radius);
      insert(n, new_node, radius);
      node->child[index] = new_node;
    } else { // else just recursively insert the body into the child node
      OctreeInternal* n = static_cast<OctreeInternal*>(child);
      insert(b, n, radius);
    }
  }
};