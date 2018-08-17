#include <cassert>
#include <iterator>
#include <utility>

template <class Monoid, template <class> class Container> class segment_tree {
public:
  using value_structure = Monoid;
  using value_type = typename value_structure::value_type;
  using container_type = Container<value_type>;
  using const_reference = typename container_type::const_reference;
  using size_type = typename container_type::size_type;

protected:
  static size_type getsize(const size_type size) {
    size_type ret = 1;
    while (ret < size)
      ret <<= 1;
    return ret;
  }

  size_type size_;
  container_type tree;

  size_type base_size() const { return tree.size() >> 1; }
  void recalc(const size_type index) {
    tree[index] =
        value_structure::operation(tree[index << 1], tree[index << 1 | 1]);
  }

public:
  segment_tree() : size_(0), tree() {}
  explicit segment_tree(const size_type size)
      : size_(size), tree(getsize(size) << 1, value_structure::identity()) {}
  template <class InputIterator>
  segment_tree(InputIterator first, InputIterator last)
      : size_(::std::distance(first, last)), tree() {
    const size_type cap = getsize(size_);
    tree.reserve(cap << 1);
    tree.resize(cap, value_structure::identity());
    tree.insert(tree.end(), first, last);
    tree.resize(cap << 1, value_structure::identity());
    for (size_type i = cap - 1; i; --i)
      recalc(i);
  }

  bool empty() const { return !size_; }
  size_type size() const { return size_; }

  const_reference operator[](const size_type index) const {
    assert(index < size());
    return tree[index + base_size()];
  }
  value_type fold(size_type first, size_type last) const {
    assert(first <= last);
    assert(first <= size());
    assert(last <= size());
    value_type ret_l = value_structure::identity(),
               ret_r = value_structure::identity();
    for (first += base_size(), last += base_size(); first < last;
         first >>= 1, last >>= 1) {
      if (first & 1)
        ret_l = value_structure::operation(::std::move(ret_l), tree[first++]);
      if (last & 1)
        ret_r = value_structure::operation(tree[last - 1], ::std::move(ret_r));
    }
    return value_structure::operation(::std::move(ret_l), ::std::move(ret_r));
  }
  template <class F> size_type search(const F &f) const {
    if (f(value_structure::identity()))
      return 0;
    if (!f(tree[1]))
      return size() + 1;
    value_type acc = value_structure::identity();
    size_type i = 1;
    while (i < base_size())
      if (!f(value_structure::operation(acc, tree[i <<= 1])))
        acc = value_structure::operation(::std::move(acc), tree[i++]);
    return i - base_size() + 1;
  }

  template <class F> void update(size_type index, const F &f) {
    assert(index < size());
    index += base_size();
    tree[index] = f(::std::move(tree[index]));
    while (index >>= 1)
      recalc(index);
  }
};

/*

verify:https://beta.atcoder.jp/contests/arc033/submissions/3018574
      :http://judge.u-aizu.ac.jp/onlinejudge/review.jsp?rid=3096017#1
      :http://judge.u-aizu.ac.jp/onlinejudge/review.jsp?rid=3096019#1
      :http://judge.u-aizu.ac.jp/onlinejudge/review.jsp?rid=3096024#1

template<class Monoid, template <class> class Container>
class segment_tree;

segment_tree はモノイドの区間和を高速に計算するデータ構造です
空間計算量 O(N)


テンプレートパラメータ
-class Monoid
 -結合律
  ∀a, ∀b, ∀c, a·(b·c) = (a·b)·c
 -単位元の存在
  ∃e, ∀a, e·a = a·e = a
 以上の条件を満たす代数的構造 (モノイド)

 以下のメンバを要求します
 -value_type
  台集合の型
 -static operation (value_type, value_type)->value_type
  2引数を取り、演算した結果を返す静的関数
 -static identity ()->value_type
  単位元を返す静的関数

-template <class> class Container
 テンプレート引数を一つ取り、それを要素とするコンテナ型
 内部実装で使用します


メンバ型
-value_structure
 構造の型 (Monoid)

-value_type
 要素となる台集合の型 (value_structure::value_type)

-container_type
 内部で使用するコンテナの型 (Container<value_type>)

-const_reference
 要素へのconst参照型 (container_type::const_reference)

-size_type
 符号なし整数型 (container_type::size_type)


メンバ関数
-(constructor) (size_type size)
 要素数 size で segment_tree を構築します
 各要素は単位元で初期化されます
 時間計算量 O(N)

template<class InputIterator>
-(constructor) (InputIterator first, InputIterator last)
 [first, last) の要素から segment_tree を構築します
 時間計算量 O(N)

-empty ()->bool
 size()==0 と同値です
 時間計算量 O(1)

-size ()->size_type
 要素数を返します
 時間計算量 O(1)

-operator[] (size_type index)->const_reference
 指定した要素にアクセスします
 時間計算量 O(1)

-fold (size_type first, size_type last)->value_type
 [first, last) の和を返します
 時間計算量 O(logN)

template<class F>
-search (F f)->size_type
 f(fold(0, i - 1)) が false を返し、
 f(fold(0, i))     が true  を返すような i を返します
 f(fold(0, -1))         は false、
 f(fold(0, size() + 1)) は true と扱います
 時間計算量 O(logN)

template<class F>
-update (size_type index, F f)
 指定した要素を f を適用した値に変更します
 時間計算量 O(logN)


※N:全体の要素数
※value_structure の各関数の時間計算量を O(1) と仮定

*/
