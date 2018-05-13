#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>

class UnionFind {

public:
  using difference_type = std::ptrdiff_t;
  using container_type = std::vector<difference_type>;
  using size_type = typename container_type::size_type;

protected:
  container_type c;

public:
  UnionFind() : c() {}
  explicit UnionFind(const size_type size) : c(size, -1) {}

  size_type size() const { return c.size(); }
  bool empty() const { return c.empty(); }

  size_type find(size_type x) {
    assert(x < size());
    while (c[x] >= static_cast<difference_type>(0)) {
      if (c[static_cast<size_type>(c[x])] >= static_cast<difference_type>(0))
        c[x] = c[static_cast<size_type>(c[x])];
      x = static_cast<size_type>(c[x]);
    }
    return x;
  }
  bool same(const size_type x, const size_type y) {
    assert(x < size());
    assert(y < size());
    return find(x) == find(y);
  }
  size_type size(const size_type x) {
    assert(x < size());
    return static_cast<size_type>(-c[find(x)]);
  }

  bool unite(size_type x, size_type y) {
    assert(x < size());
    assert(y < size());
    x = find(x);
    y = find(y);
    if (x == y)
      return false;
    if (c[x] > c[y])
      std::swap(x, y);
    c[x] += c[y];
    c[y] = static_cast<difference_type>(x);
    return true;
  }
};

/*

verify:https://beta.atcoder.jp/contests/atc001/submissions/2509045
      :http://judge.u-aizu.ac.jp/onlinejudge/review.jsp?rid=2860145#1

class UnionFind;

UnionFindは素集合を管理するデータ構造です
空間計算量 O(N)


メンバ型
-difference_type
 符号あり整数型 (std::ptrdiff_t)
 内部実装で使用

-container_type
 内部で使用するコンテナ型 (std::vector<difference_type>)

-size_type
 符号なし整数型 (container_type::size_type)


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