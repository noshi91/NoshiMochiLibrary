#include <cassert>
#include <cstdint>
#include <tuple>
#include <vector>

template <typename Abelian> class PotentializedUnionFind {

public:
  using value_type = Abelian;
  using reference = value_type &;
  using const_reference = const value_type &;
  using size_type = std::uint_fast32_t;

private:
  std::vector<std::tuple<bool, size_type, value_type>> tree;
  value_type potential(const size_type x) {
    find(x);
    return std::get<2>(tree[x]);
  }

public:
  PotentializedUnionFind(const size_type size)
      : tree(size, std::forward_as_tuple(1, 1, value_type())) {}
  size_type find(const size_type x) {
    assert(x < size());
    if (std::get<0>(tree[x]))
      return x;
    const size_type t = std::get<1>(tree[x]);
    std::get<1>(tree[x]) = find(t);
    std::get<2>(tree[x]) = std::get<2>(tree[t]) + std::get<2>(tree[x]);
    return std::get<1>(tree[x]);
  }
  value_type diff(const size_type x, const size_type y) {
    assert(x < size());
    assert(y < size());
    return potential(y) + (-potential(x));
  }
  bool unite(size_type x, size_type y, value_type d) {
    assert(x < size());
    assert(y < size());
    d = d + diff(y, x);
    x = find(x);
    y = find(y);
    if (x == y)
      return false;
    if (std::get<1>(tree[x]) < std::get<1>(tree[y]))
      std::swap(x, y), d = -d;
    std::get<1>(tree[x]) += std::get<1>(tree[y]);
    tree[y] = std::forward_as_tuple(0, x, d);
    return true;
  }
  bool same(const size_type x, const size_type y) {
    assert(x < size());
    assert(y < size());
    return find(x) == find(y);
  }
  size_type size(const size_type x) {
    assert(x < size());
    return std::get<1>(tree[find(x)]);
  }
  size_type size() const noexcept { return tree.size(); }
  bool empty() const noexcept { return tree.empty(); }
};

/*

verify:https://beta.atcoder.jp/contests/abc087/submissions/2298400

template <typename Abelian>
class PotentializedUnionFind;

PotentializedUnionFindはポテンシャルが付いた要素からなる素集合を管理するデータ構造です
空間計算量 O(N)


テンプレートパラメータ
-typename Abelian
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

-reference
 要素(value_type)への参照型 (value_type &)

-const_reference
 要素(value_type)へのconst参照型 (const value_type &)

-size_type
 符号なし整数型 (std::uint_fast32_t)


メンバ関数
-(constructor) (size_type size)
 独立した要素を size 個持つ状態で構築します
 時間計算量 O(N)

-find (size_type x)->size_type
 x の根を返します
 時間計算量 償却 O(α(N))

-diff (size_type x, size_type y)->value_type
 x を基準とした y のポテンシャルを返します
 x と y が異なる集合に属していた場合の動作は保証されません
 時間計算量 償却 O(α(N))

-unite (size_type x, size_type y, value_type d)->bool
 x と y がそれぞれ含まれる集合を x を基準とした y のポテンシャルが
 d となるように併合します
 x と y が既に同じ集合に属していた場合、ポテンシャルは変化しません
 併合に成功したか、すなわち x と y が違う集合に属していたかを真偽値で返します
 時間計算量 償却 O(α(N))

-same (size_type x, size_type y)->bool
 x と y が同じ集合に属しているかを真偽値で返します
 時間計算量 償却 O(α(N))

-size (size_type x)->size_type
 x の含まれる集合に含まれる要素数を返します
 時間計算量 償却 O(α(N))

-size ()->size_type
 全体の要素数を返します
 時間計算量 O(1)

-empty()->bool
 全体の集合が空であるかどうかを真偽値で返します
 時間計算量 O(1)


※N:全体の要素数
※α():アッカーマン関数の逆関数
※operator+, operator-の時間計算量を O(1) と仮定

*/