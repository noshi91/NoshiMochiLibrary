#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>

template <class Abelian> class PotentializedUnionFind {

public:
  using value_type = Abelian;
  using difference_type = std::ptrdiff_t;
  using container_type = std::vector<std::pair<difference_type, value_type>>;
  using size_type = typename container_type::size_type;

protected:
  container_type c;

private:
  value_type potential(size_type x) {
    value_type ret = {};
    while (c[x].first >= static_cast<difference_type>(0)) {
      if (c[static_cast<size_type>(c[x].first)].first >=
          static_cast<difference_type>(0)) {
        c[x].second =
            c[static_cast<size_type>(c[x].first)].second + c[x].second;
        c[x].first = c[static_cast<size_type>(c[x].first)].first;
      }
      ret = ret + c[x].second;
      x = static_cast<size_type>(c[x].first);
    }
    return std::move(ret);
  }

public:
  PotentializedUnionFind() : c() {}
  explicit PotentializedUnionFind(const size_type size)
      : c(size, std::make_pair(-1, value_type())) {}

  size_type size() const { return c.size(); }
  bool empty() const { return c.empty(); }

  size_type find(size_type x) {
    assert(x < size());
    while (c[x].first >= static_cast<difference_type>(0)) {
      if (c[static_cast<size_type>(c[x].first)].first >=
          static_cast<difference_type>(0)) {
        c[x].second =
            c[static_cast<size_type>(c[x].first)].second + c[x].second;
        c[x].first = c[static_cast<size_type>(c[x].first)].first;
      }
      x = static_cast<size_type>(c[x].first);
    }
    return x;
  }
  value_type diff(const size_type x, const size_type y) {
    assert(x < size());
    assert(y < size());
    return potential(y) + (-potential(x));
  }
  bool same(const size_type x, const size_type y) {
    assert(x < size());
    assert(y < size());
    return find(x) == find(y);
  }
  size_type size(const size_type x) {
    assert(x < size());
    return static_cast<size_type>(-c[find(x)].first);
  }

  bool unite(size_type x, size_type y, value_type d) {
    assert(x < size());
    assert(y < size());
    d = d + diff(y, x);
    x = find(x);
    y = find(y);
    if (x == y)
      return false;
    if (c[x].first > c[y].first)
      std::swap(x, y), d = -d;
    c[x].first += c[y].first;
    c[y] = std::make_pair(static_cast<difference_type &&>(x), std::move(d));
    return true;
  }
};

/*

verify:https://beta.atcoder.jp/contests/abc087/submissions/2509195
      :http://judge.u-aizu.ac.jp/onlinejudge/review.jsp?rid=2860228#1

template <class Abelian>
class PotentializedUnionFind;

PotentializedUnionFindはポテンシャルが付いた要素からなる素集合を管理するデータ構造です
空間計算量 O(N)


テンプレートパラメータ
-class Abelian
 結合律 ∀a, ∀b, ∀c, a + (b + c) = (a + b) + c
 交換律 ∀a, ∀b, a + b = b + a
 単位元 ∃e, ∀a, e + a = a + e = a
 逆元　 ∀a, ∃-a, a + (-a) = e
 以上の条件を満たす代数的構造 (アーベル群)

 -加法   :operator+(2項)
 -単位元 :デフォルトコンストラクタ
 -逆元   :operator-(単項)
  以上のように定義されている必要があります


メンバ型
-value_type
 要素の型 (Abelian)

-difference_type
 符号あり整数型 (std::ptrdiff_t)
 内部実装で使用

-container_type
 内部で使用するコンテナ型

-size_type
 符号なし整数型


メンバ関数
-(constructor) (size_type size)
 独立した要素を size 個持つ状態で構築します
 時間計算量 O(N)

-size ()->size_type
 全体の要素数を返します
 時間計算量 O(1)

-empty()->bool
 全体の集合が空であるかどうかを真偽値で返します
 時間計算量 O(1)

-find (size_type x)->size_type
 x の根を返します
 時間計算量 償却 O(α(N))

-diff (size_type x, size_type y)->value_type
 x を基準とした y のポテンシャルを返します
 x と y が異なる集合に属していた場合の動作は保証されません
 時間計算量 償却 O(α(N))

-same (size_type x, size_type y)->bool
 x と y が同じ集合に属しているかを真偽値で返します
 時間計算量 償却 O(α(N))

-size (size_type x)->size_type
 x の含まれる集合に含まれる要素数を返します
 時間計算量 償却 O(α(N))

-unite (size_type x, size_type y, value_type d)->bool
 x と y がそれぞれ含まれる集合を x を基準とした y のポテンシャルが
 d となるように併合します
 x と y が既に同じ集合に属していた場合、ポテンシャルは変化しません
 併合に成功したか、すなわち x と y が違う集合に属していたかを真偽値で返します
 時間計算量 償却 O(α(N))


※N:全体の要素数
※α():アッカーマン関数の逆関数
※operator+, operator-の時間計算量を O(1) と仮定

*/