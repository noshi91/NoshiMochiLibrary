#include <cassert>
#include <cstddef>
#include <numeric>
#include <utility>
#include <vector>

class UnionFind {

public:
  using size_type = std::size_t;
  using container_type = std::vector<size_type>;

protected:
  container_type p, s;

public:
  UnionFind() : p(), s() {}
  explicit UnionFind(const size_type size)
      : p(size), s(size, static_cast<size_type>(1)) {
    std::iota(p.begin(), p.end(), static_cast<size_type>(0));
  }

  size_type size() const { return p.size(); }
  bool empty() const { return p.empty(); }

  size_type find(size_type x) {
    assert(x < size());
    while (p[x] != x)
      x = p[x] = p[p[x]];
    return x;
  }
  bool same(const size_type x, const size_type y) {
    assert(x < size());
    assert(y < size());
    return find(x) == find(y);
  }
  size_type size(const size_type x) {
    assert(x < size());
    return s[find(x)];
  }

  bool unite(size_type x, size_type y) {
    assert(x < size());
    assert(y < size());
    x = find(x);
    y = find(y);
    if (x == y)
      return false;
    if (s[x] < s[y])
      std::swap(x, y);
    s[x] += s[y];
    p[y] = x;
    return true;
  }
};

/*

verify:https://beta.atcoder.jp/contests/atc001/submissions/2525740
      :http://judge.u-aizu.ac.jp/onlinejudge/review.jsp?rid=2873775#1

class UnionFind;

UnionFindは素集合を管理するデータ構造です
空間計算量 O(N)


メンバ型
-size_type
 符号なし整数型 (std::size_t)

-container_type
 内部で使用するコンテナ型 (std::vector<size_type>)


メンバ関数
-(constructor) (size_type size)
 独立した要素を size 個持つ状態で構築します
 時間計算量 O(N)

-size ()->size_type
 全体の要素数を返します
 時間計算量 O(1)

-empty ()->bool
 全体の集合が空であるかを真偽値で返します
 時間計算量 O(1)

-find (size_type x)->size_type
 x の根を返します
 時間計算量 償却 O(α(N))

-same (size_type x, size_type y)->bool
 x と y が同じ集合に属しているかを真偽値で返します
 時間計算量 償却 O(α(N))

-size (size_type x)->size_type
 x の含まれる集合に含まれる要素数を返します
 時間計算量 償却 O(α(N))

-unite (size_type x, size_type y)->bool
 x と y がそれぞれ含まれる集合を併合します
 併合に成功したか、すなわち x と y が違う集合に属していたかを真偽値で返します
 時間計算量 償却 O(α(N))


※N:全体の要素数
※α():アッカーマン関数の逆関数

*/