/**
 * A node in an octree is either an internal node or a body (leaf).
 */
struct Octree {
  virtual ~Octree() { }
  virtual bool isLeaf() const = 0;
};

struct OctreeInternal : Octree {
  Octree* child[8];
  Point pos;
  double mass;
  OctreeInternal(Point _pos) : pos(_pos), mass(0.0) {
    bzero(child, sizeof(*child) * 8);
  }
  virtual ~OctreeInternal() {
    for (int i = 0; i < 8; i++) {
      if (child[i] != NULL && !child[i]->isLeaf()) {
        delete child[i];
      }
    }
  }
  virtual bool isLeaf() const {
    return false;
  }
};

struct Body : Octree {
  Point pos;
  Point vel;
  Point acc;
  double mass;
  Body() { }
  virtual bool isLeaf() const {
    return true;
  }
};

std::ostream& operator<<(std::ostream& os, const Body& b) {
  os << "(pos:" << b.pos
     << " vel:" << b.vel
     << " acc:" << b.acc
     << " mass:" << b.mass << ")";
  return os;
}