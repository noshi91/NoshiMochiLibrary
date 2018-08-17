#include <cassert>
#include <utility>

template <class Group, template <class> class Container>
class potentialized_union_find {
protected:
  class node_type;

public:
  using value_structure = Group;
  using value_type = typename value_structure::value_type;
  using container_type = Container<node_type>;
  using size_type = typename container_type::size_type;

protected:
  class node_type {
  public:
    typename potentialized_union_find::size_type parent;
    typename potentialized_union_find::value_type value;
    typename potentialized_union_find::size_type size;
  };

  container_type tree;

private:
  value_type potential(size_type x) {
    value_type ret = value_structure::identity();
    while (tree[x].parent != x) {
      tree[x].value =
          value_structure::operation(tree[tree[x].parent].value, tree[x].value);
      ret = value_structure::operation(tree[x].value, ret);
      x = tree[x].parent = tree[tree[x].parent].parent;
    }
    return ::std::move(ret);
  }

public:
  potentialized_union_find() : tree() {}
  explicit potentialized_union_find(const size_type size)
      : tree(size, {0, value_structure::identity(), 1}) {
    for (size_type i = 0; i < size; ++i)
      tree[i].parent = i;
  }

  bool empty() const { return tree.empty(); }
  size_type size() const { return tree.size(); }

  size_type find(size_type x) {
    assert(x < size());
    while (tree[x].parent != x) {
      tree[x].value =
          value_structure::operation(tree[tree[x].parent].value, tree[x].value);
      x = tree[x].parent = tree[tree[x].parent].parent;
    }
    return x;
  }
  value_type diff(const size_type x, const size_type y) {
    assert(x < size());
    assert(y < size());
    assert(same(x, y));
    return value_structure::operation(value_structure::inverse(potential(x)),
                                      potential(y));
  }
  bool same(const size_type x, const size_type y) {
    assert(x < size());
    assert(y < size());
    return find(x) == find(y);
  }
  size_type size(const size_type x) {
    assert(x < size());
    return tree[find(x)].size;
  }

  ::std::pair<size_type, size_type> unite(size_type x, size_type y,
                                          value_type d) {
    assert(x < size());
    assert(y < size());
    d = value_structure::operation(
        value_structure::operation(potential(x), ::std::move(d)),
        value_structure::inverse(potential(y)));
    x = find(x);
    y = find(y);
    if (x != y) {
      if (tree[x].size < tree[y].size) {
        ::std::swap(x, y);
        d = value_structure::inverse(::std::move(d));
      }
      tree[x].size += tree[y].size;
      tree[y].parent = x;
      tree[y].value = ::std::move(d);
    }
    return {x, y};
  }
};

/*

verify:https://beta.atcoder.jp/contests/abc087/submissions/3018497
      :http://judge.u-aizu.ac.jp/onlinejudge/review.jsp?rid=3095975#1

template <class Group, template <class> class Container>
class potentialized_union_find;

potentialized_union_find はポテンシャルが付いた要素からなる
素集合を管理するデータ構造です
空間計算量 O(N)


テンプレートパラメータ
-class Group
 -結合律
  ∀a, ∀b, ∀c, a·(b·c) = (a·b)·c
 -単位元の存在
  ∃e, ∀a, e·a = a·e = a
 -逆元の存在
  ∀a, ∃a^-1, a·(a^-1) = (a^-1)·a = e
 以上の条件を満たす代数的構造 (群)

 以下のメンバを要求します
 -value_type
  台集合の型
 -static operation (value_type, value_type)->value_type
  2引数を取り、演算した結果を返す静的関数
 -static identity ()->value_type
  単位元を返す静的関数
 -static inverse (value_type)->value_type
  1引数を取り、逆元を返す静的関数

-template <class> class Container
 型引数を1つ取り、それを要素とするコンテナ型
 内部実装で使用します


メンバ型
-value_structure
 構造の型 (Group)

-value_type
 要素となる台集合の型 (value_structure::value_type)

-container_type
 内部で使用するコンテナ型

-size_type
 符号なし整数型 (container_type::size_type)


メンバ関数
-(constructor) (size_type size)
 独立した要素を size 個持つ状態で構築します
 時間計算量 O(N)

-empty()->bool
 全体の集合が空であるかどうかを真偽値で返します
 時間計算量 O(1)

-size ()->size_type
 全体の要素数を返します
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
 併合前の x, y それぞれの根を、併合後に根となったほうを第一要素として返します
 時間計算量 償却 O(α(N))


※N:全体の要素数
※α():アッカーマン関数 A(n, n) の逆関数
※value_structure の各関数の時間計算量を O(1) と仮定

*/