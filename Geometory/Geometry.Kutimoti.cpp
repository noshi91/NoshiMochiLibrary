
/*

DONT USE NOW!!!

*/


#include <algorithm>
#include <cmath>
#include <tuple>
#include <vector>
using namespace std;

using ld = long double;

const ld EPS = 1e-8;
inline bool eq(ld a, ld b) { return abs(a - b) < EPS; }
const ld PI = acos(-1);

struct Geometory {
  struct Point {
    ld x, y;
    Point(ld x = 0, ld y = 0) : x(x), y(y) {}
    Point operator+(const Point& b) const { return Point(x + b.x, y + b.y); }
    Point operator-(const Point& b) const { return Point(x - b.x, y - b.y); }
    Point operator*(const ld b) const { return Point(x * b, y * b); }
    Point operator/(const ld b) const { return Point(x / b, y / b); }
    bool operator<(const Point& b) const {
      if (x != b.x)
        return x < b.x;
      else
        return y < b.y;
    }
    bool operator==(const Point& b) const { return eq(x, b.x) && eq(y, b.y); }
    ld norm() const { return x * x + y * y; }
    ld abs() const { return sqrt(norm()); }
    ld arg() const { return atan2(x, y); }
    Point rotate(const ld theta) const {
      ld co = cos(theta);
      ld si = sin(theta);
      return Point(co * x - si * y, si * x + y * co);
    }
    Point rotate90() const { return Point(-y, x); }
  };

  ld dot(const Point& a, const Point& b) { return a.x * b.x + a.y * b.y; }
  ld cross(const Point& a, const Point b) { return a.x * b.y - a.y * b.x; }

  struct Line {
    Point from, to;
    Line(Point from = Point(), Point to = Point()) : from(from), to(to) {}
  };

  struct Segment {
    Point from, to;
    Segment(Point from = Point(), Point to = Point()) : from(from), to(to) {}
  };

  bool is_orthogonal(const Line& la, const Line& lb) {
    return eq(0.0, dot(la.from - la.to, lb.from - lb.from));
  }
  bool is_parallel(const Line& la, const Line& lb) {
    return eq(0.0, cross(la.from - la.to, lb.from - lb.from));
  }
  bool is_Point_on(const Line& l, const Point& p) {
    return eq(0.0, cross(l.to - l.from, p - l.from));
  }
  bool is_Point_on(const Segment& s, const Point& p) {
    return (s.from - p).abs() + (p - s.to).abs() < (s.from - s.to).abs() + EPS;
  }
  ld distance(const Line& l, const Point& p) {
    return abs(cross(l.to - l.from, p - l.from)) / (l.to - l.from).abs();
  }
  ld distance(const Segment& s, const Point& p) {
    if (dot(s.to - s.from, p - s.from) < EPS) return (p - s.from).abs();
    if (dot(s.from - s.to, p - s.to) < EPS) return (p - s.to).abs();
    return abs(cross(s.to - s.from, p - s.from)) / (s.to - s.from).abs();
  }
  ld is_intersected(const Segment& a, const Segment& b) {
    return (cross(a.to - a.from, b.from - a.from) *
                cross(a.to - a.from, b.to - a.from) <
            EPS) &&
           (cross(b.to - b.from, a.from - b.from) *
                cross(b.to - b.from, a.to - b.from) <
            EPS);
  }

  ld is_intersected(const Segment& s, const Line& l) {
    // line -> ax + by + c = 0
    ld a = l.to.y - l.from.y;
    ld b = l.from.x - l.to.x;
    ld c = -a * l.from.x - b * l.from.y;
    ld t1 = a * s.from.x + b * s.from.y + c;
    ld t2 = a * s.to.x + b * s.to.y + c;
    return t1 * t2 <= 0;
  }

  Point intersection_point(const Segment& a, const Segment& b) {
    Point bp = b.to - b.from;
    ld d1 = abs(cross(bp, a.from - b.from));
    ld d2 = abs(cross(bp, a.to - b.from));
    ld t = d1 / (d1 + d2);
    return a.from + (a.to - a.from) * t;
  }

  Point intersection_point(const Line& a, const Line& b) {
    Point ap = a.to - a.from;
    Point bp = b.to - b.from;
    return a.from + ap * cross(bp, b.from - a.from) / cross(bp, ap);
  }
  // counterclockwise
  int ccw(const Point& a, const Point& b, const Point& c) {
    Point ba = b - a;
    Point ca = c - a;
    if (cross(ba, ca) > EPS) return 1;   // a - b --/ c
    if (cross(ba, ca) < EPS) return -1;  // a - b --| c
    if (dot(ba, ca) < 0) return 2;       // b - a - c
    if (b.norm() < c.norm()) return -2;  // a - b - c
    return 0;                            // a -- c -- b
  }

  vector<Point> Convex_Hull(vector<Point>& p) {
    int n = p.size();
    int k = 0;
    if (n >= 3) {
      sort(p.begin(), p.end());
      vector<Point> ch(2 * n);
      for (int i = 0; i < n; ch[k++] = p[i++]) {
        while (k >= 2 && cross(ch[k - 1] - ch[k - 2], p[i] - ch[k - 1]) < 0)
          k--;
      }
      for (int i = n - 2, t = k + 1; i >= 0; ch[k++] = p[i--]) {
        while (k >= t && cross(ch[k - 1] - ch[k - 2], p[i] - ch[k - 1]) < 0)
          k--;
      }
      ch.resize(k - 1);
      return ch;
    } else {
      return p;
    }
  }
};
