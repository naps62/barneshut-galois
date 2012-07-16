/**
 * Defines a 3D box based only on 2 points
 */ 
struct BoundingBox {
  Point min;
  Point max;

  /**
   * Inits the box with a single point, equal to both min and max
   */
  explicit BoundingBox(const Point& p) : min(p), max(p) { }

  /**
   * If not args, box is infinite
   */
  BoundingBox() :
    min(std::numeric_limits<double>::max()),
    max(std::numeric_limits<double>::min()) { }

  /**
   * Given another box, merges both, by giving the smallest boundingBox that wraps both of them
   * 
   * eg:
   * b1 =           ((0,0,0),   (1,1,1))
   * b2 =           ((2,2,2),   (3,3,3))
   *
   * b1.merge(b2) = ((0,0,0),   (3,3,3))
   */
  void merge(const BoundingBox& other) {
    for (int i = 0; i < 3; i++) {
      if (other.min[i] < min[i])
        min[i] = other.min[i];
    }
    for (int i = 0; i < 3; i++) {
      if (other.max[i] > max[i])
        max[i] = other.max[i];
    }
  }

  /**
   * Same thing as above, but merges the box with a single point
   * Basically expands the box up to the fiven point
   */
  void merge(const Point& other) {
    for (int i = 0; i < 3; i++) {
      if (other[i] < min[i])
        min[i] = other[i];
    }
    for (int i = 0; i < 3; i++) {
      if (other[i] > max[i])
        max[i] = other[i];
    }
  }

  /**
   * Don't understand it? You don't deserve this comment
   */
  double diameter() const {
    double diameter = max.x - min.x;
    for (int i = 1; i < 3; i++) {
      double t = max[i] - min[i];
      if (diameter < t)
        diameter = t;
    }
    return diameter;
  }

  /**
   * Why are you even looking for info here? it's a radius!
   */
  double radius() const {
    return diameter() / 2;
  }

  /**
   * This comment was left empty on purpose
   */
  Point center() const {
    return Point(
        (max.x + min.x) * 0.5,
        (max.y + min.y) * 0.5,
        (max.z + min.z) * 0.5);
  }
};

std::ostream& operator<<(std::ostream& os, const BoundingBox& b) {
  os << "(min:" << b.min << " max:" << b.max << ")";
  return os;
}
