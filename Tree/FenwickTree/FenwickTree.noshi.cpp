#include <cassert>
#include <utility>

template <class CommutativeMonoid, template <class> class Container>
class fenwick_tree {

public:
  using value_structure = CommutativeMonoid;
  using value_type = typename value_structure::value_type;
  using container_type = Container<value_type>;
  using const_reference = typename container_type::const_reference;
  using size_type = typename container_type::size_type;

private:
  size_type base_size;
  container_type tree;
  static size_type getsize(const size_type size) {
    size_type ret = 1;
    while (ret < size)
      ret <<= 1;
    return ret;
  }

public:
  fenwick_tree() : base_size(0), tree() {}
  explicit fenwick_tree(const size_type size)
      : base_size(getsize(size)), tree(size + 1, value_structure::identity()) {}

  bool empty() const { return size() == 0; }
  size_type size() const { return tree.size() - 1; }

  value_type fold(size_type last) const {
    assert(last <= size());
    value_type ret = value_structure::identity();
    for (; last; last &= last - 1)
      ret = value_structure::operation(tree[last], ::std::move(ret));
    return ret;
  }
  template <class F> size_type search(const F &f) const {
    if (f(value_structure::identity()))
      return 0;
    size_type i = 0, k = base_size;
    value_type acc = value_structure::identity();
    while (k >>= 1)
      if ((i | k) < tree.size() &&
          !f(value_structure::operation(acc, tree[i | k])))
        acc = value_structure::operation(::std::move(acc), tree[i |= k]);
    return i + 1;
  }

  void add(size_type index, const value_type &value) {
    assert(index < size());
    for (++index; index < tree.size(); index += index & ~index + 1)
      tree[index] = value_structure::operation(::std::move(tree[index]), value);
  }
};

/*

verify:https://beta.atcoder.jp/contests/arc033/submissions/3018652
      :http://judge.u-aizu.ac.jp/onlinejudge/review.jsp?rid=3096064#1

template<class CommutativeMonoid, template <class> class Container>
class fenwick_tree;

fenwick_tree は可換モノイドの区間和を高速に計算するデータ構造です
空間計算量 O(N)


テンプレートパラメータ
-class CommutativeMonoid
 -結合律
  ∀a, ∀b, ∀c, a·(b·c) = (a·b)·c
 -交換律
  ∀a, ∀b, a·b = b·a
 -単位元の存在
  ∃e, ∀a, e·a = a·e = a
 以上の条件を満たす代数的構造 (可換モノイド)

 以下のメンバを要求します
 -value_type
  台集合の型
 -static operation (value_type, value_type)->value_type
  2引数を取り、演算した結果を返す静的関数
 -static identity ()->value_type
  単位元を返す静的関数

-template <class> class Container
 型引数を1つ取り、それを要素とするコンテナ型
 内部実装で使用します


メンバ型
-value_structure
 構造の型 (CommutativeMonoid)

-value_type
 要素となる台集合の型 (value_structure::value_type)

-container_type
 コンテナの型 (Container<value_type>)

-size_type
 符号なし整数型 (container_type::size_type)


メンバ関数
-(constructor) (size_type size)
 size 個の要素からなる fenwick_tree を構築します
 各要素は単位元で初期化されます
 時間計算量 O(N)

-empty ()->bool
 size()==0 と等価です
 時間計算量 O(1)

-size ()->size_type
 要素数を返します
 時間計算量 O(1)

-fold (size_type last)-value_type
 [0, last) の和を返します
 last == 0 のとき 単位元を返します
 時間計算量 O(logN)

template<class F>
-search (F f)->size_type
 f(fold(i - 1)) が false を返し、
 f(fold(i))     が true  を返すような i を返します
 f(fold(-1))         は false、
 f(fold(size() + 1)) は true と扱います
 時間計算量 O(logN)
 
-update (size_type index, const value_type &value)
 index で指定した要素に value を加算します
 時間計算量 O(logN)


※N:全体の要素数
※value_structure の各関数の時間計算量を O(1) と仮定

*/
