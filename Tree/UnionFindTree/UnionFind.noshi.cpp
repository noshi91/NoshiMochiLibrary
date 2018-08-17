#include <cassert>
#include <utility>

template <template <class> class Container> class union_find {
protected:
  class node_type;

public:
  using container_type = Container<node_type>;
  using size_type = typename container_type::size_type;

protected:
  class node_type {
  public:
    typename union_find::size_type parent;
    typename union_find::size_type size;
  };

  container_type tree;

public:
  union_find() : tree() {}
  explicit union_find(const size_type size) : tree(size, {0, 1}) {
    for (size_type i = 0; i < size; ++i)
      tree[i].parent = i;
  }

  bool empty() const { return tree.empty(); }
  size_type size() const { return tree.size(); }

  size_type find(size_type x) {
    assert(x < size());
    while (tree[x].parent != x)
      x = tree[x].parent = tree[tree[x].parent].parent;
    return x;
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

  ::std::pair<size_type, size_type> unite(size_type x, size_type y) {
    assert(x < size());
    assert(y < size());
    x = find(x);
    y = find(y);
    if (x != y) {
      if (tree[x].size < tree[y].size)
        ::std::swap(x, y);
      tree[x].size += tree[y].size;
      tree[y].parent = x;
    }
    return {x, y};
  }
};

/*

verify:https://beta.atcoder.jp/contests/atc001/submissions/3018421
      :http://judge.u-aizu.ac.jp/onlinejudge/review.jsp?rid=3095943#1

template<template <class> class Container>
class union_find;

union_find は素集合を管理するデータ構造です
空間計算量 O(N)

テンプレートパラメータ
-template <class> class Container
 型引数を1つ取り、それを要素とするコンテナ型
 内部実装で使用する


メンバ型
-container_type
 内部で使用するコンテナ型

-size_type
 符号なし整数型 (container_type::size_type)


メンバ関数
-(constructor) (size_type size)
 独立した要素を size 個持つ状態で構築します
 時間計算量 O(N)

-empty ()->bool
 全体の集合が空であるかを真偽値で返します
 時間計算量 O(1)

-size ()->size_type
 全体の要素数を返します
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

-unite (size_type x, size_type y)->::std::pair<size_type, size_type>
 x と y がそれぞれ含まれる集合を併合します
 併合前の x, y それぞれの根を、併合後に根となったほうを第一要素として返します
 時間計算量 償却 O(α(N))


※N:全体の要素数
※α():アッカーマン関数 A(n, n) の逆関数

*/