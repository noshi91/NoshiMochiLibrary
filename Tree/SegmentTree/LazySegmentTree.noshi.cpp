#include <cassert>
#include <iterator>
#include <stdexcept>
#include <utility>

template <class ValueMonoid, class OperatorMonoid, class Modifier,
          template <class> class Container>
class lazy_segment_tree {
public:
  using value_structure = ValueMonoid;
  using value_type = typename value_structure::value_type;
  using const_reference = const value_type &;
  using operator_structure = OperatorMonoid;
  using operator_type = typename operator_structure::value_type;
  using modifier = Modifier;
  using container_type = Container<::std::pair<value_type, operator_type>>;
  using size_type = typename container_type::size_type;

private:
  size_type size_, height;
  container_type tree;
  static size_type getheight(const size_type size) noexcept {
    size_type ret = 0;
    while (static_cast<size_type>(1) << ret < size)
      ++ret;
    return ret;
  }
  static value_type reflect(typename container_type::const_reference element) {
    return modifier::operation(element.first, element.second);
  }
  void recalc(const size_type index) {
    tree[index].first = value_structure::operation(
        reflect(tree[index << 1]), reflect(tree[index << 1 | 1]));
  }
  static void assign(operator_type &element, const operator_type &data) {
    element = operator_structure::operation(element, data);
  }
  void push(const size_type index) {
    assign(tree[index << 1].second, tree[index].second);
    assign(tree[index << 1 | 1].second, tree[index].second);
    tree[index].second = operator_structure::identity();
  }
  void propagate(const size_type index) {
    for (size_type i = height; i; --i)
      push(index >> i);
  }
  void thrust(const size_type index) {
    tree[index].first = reflect(tree[index]);
    push(index);
  }
  void evaluate(const size_type index) {
    for (size_type i = height; i; --i)
      thrust(index >> i);
  }
  void build(size_type index) {
    while (index >>= 1)
      recalc(index);
  }
  size_type base_size() const { return static_cast<size_type>(1) << height; }

public:
  lazy_segment_tree() : size_(0), height(0), tree() {}
  explicit lazy_segment_tree(const size_type size)
      : size_(size), height(getheight(size_)),
        tree(static_cast<size_type>(1) << (height + 1),
             {value_structure::identity(), operator_structure::identity()}) {}
  template <class InputIterator>
  explicit lazy_segment_tree(InputIterator first, InputIterator last)
      : size_(::std::distance(first, last)), height(getheight(size_)), tree() {
    const size_type cap = static_cast<size_type>(1) << height;
    tree.reserve(cap << 1);
    tree.resize(cap,
                {value_structure::identity(), operator_structure::identity()});
    for (; first != last; ++first)
      tree.emplace_back(*first, operator_structure::identity());
    tree.resize(cap << 1,
                {value_structure::identity(), operator_structure::identity()});
    for (size_type i = cap - 1; i; --i)
      recalc(i);
  }

  bool empty() const { return !size_; }
  size_type size() const { return size_; }

  const_reference operator[](size_type index) {
    assert(index < size());
    index += base_size();
    evaluate(index);
    tree[index].first = reflect(tree[index]);
    tree[index].second = operator_structure::identity();
    return tree[index].first;
  }
  const_reference at(size_type index) {
    if (index < size()) {
      throw ::std::out_of_range("index out of range");
    } else {
      index += base_size();
      evaluate(index);
      tree[index].first = reflect(tree[index]);
      tree[index].second = operator_structure::identity();
      return tree[index].first;
    }
  }
  value_type fold(size_type first, size_type last) {
    assert(first <= last);
    assert(first <= size());
    assert(last <= size());
    first += base_size();
    last += base_size();
    evaluate(first);
    evaluate(last - 1);
    value_type ret_l = value_structure::identity(),
               ret_r = value_structure::identity();
    for (; first < last; first >>= 1, last >>= 1) {
      if (first & 1)
        ret_l = value_structure::operation(ret_l, reflect(tree[first++]));
      if (last & 1)
        ret_r = value_structure::operation(reflect(tree[last - 1]), ret_r);
    }
    return value_structure::operation(ret_l, ret_r);
  }
  template <class F> size_type search(const F &f) {
    if (f(value_structure::identity()))
      return static_cast<size_type>(0);
    if (!f(reflect(tree[1])))
      return size() + 1;
    value_type acc = value_structure::identity();
    size_type i = 1;
    while (i < base_size()) {
      thrust(i);
      if (!f(value_structure::operation(acc, reflect(tree[i <<= 1]))))
        acc = value_structure::operation(acc, reflect(tree[i++]));
    }
    return i - base_size() + 1;
  }

  template <class F> void update(size_type index, const F &f) {
    assert(index < size());
    index += base_size();
    propagate(index);
    tree[index].first = f(reflect(tree[index]));
    tree[index].second = operator_structure::identity();
    build(index);
  }
  void update(size_type first, size_type last, const operator_type &data) {
    assert(first <= last);
    assert(first <= size());
    assert(last <= size());
    first += base_size();
    last += base_size();
    propagate(first);
    propagate(last - 1);
    for (size_type left = first, right = last; left < right;
         left >>= 1, right >>= 1) {
      if (left & 1)
        assign(tree[left++].second, data);
      if (right & 1)
        assign(tree[right - 1].second, data);
    }
    build(first);
    build(last - 1);
  }
};

/*

verify:http://judge.u-aizu.ac.jp/onlinejudge/review.jsp?rid=3096104#1
      :http://judge.u-aizu.ac.jp/onlinejudge/review.jsp?rid=3096125#1
      :http://judge.u-aizu.ac.jp/onlinejudge/review.jsp?rid=3096141#1

template<class ValueMonoid, class OperatorMonoid, class Modify,
         template <class> class Container>
         class lazy_segment_tree;

lazy_segment_tree はモノイドの区間和と区間更新を高速に計算するデータ構造です
空間計算量 O(N)


テンプレートパラメータ
-class ValueMonoid
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

-class OperatorMonoid
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

-class Modify
 (ValueMonoid の台集合を V、OperatorMonoid の台集合を O、Modify を m と表記)
 -閉性
  ∀a∈V, ∀b∈O, m(a, b)∈V
 -結合律
  ∀a∈V, ∀b∈O, ∀c∈O, m(a, b·c) = m(m(a, b), c)
 -分配法則
  ∀a∈V, ∀b∈V, ∀c∈O, m(a, c)·m(b, c) = m(a·b, c)
 以上の条件を満たすように ValueMonoid に OperatorMonoid を作用させるクラス

 以下のメンバを要求します
 -static operation (ValueMonoid::value_type, OperatorMonoid::value_type)
                    ->ValueMonoid::value_type
  2引数を取り、作用させた結果を返す静的関数
 
-template <class> class Container
 型引数を1つ取り、それを要素とするコンテナ型
 内部実装で使用します


メンバ型
-value_structure
 要素の構造の型 (ValueMonoid)

-value_type
 要素の型 (value_structure::value_type)

-const_reference
 要素(value_type)へのconst参照型 (const value_type &)

-operator_structure
 演算要素の構造の型 (OperatorMonoid)

-operator_type
 演算要素の型 (operator_structure::value_type)

-container_type
 内部実装のコンテナ型 (Container<::std::pair<value_type, operator_type>>)

-size_type
 符号なし整数型 (container_type::size_type)


メンバ関数
-(constructor) (size_type size)
 要素数 size の lazy_segment_tree を構築します
 各要素は単位元で初期化されます
 時間計算量 O(N)

template<class InputIterator>
-(constructor) (InputIterator first, InputIterator last)
 [first, last) の要素から lazy_segment_tree を構築します
 時間計算量 O(N)


-empty ()->bool
 size()==0 と同値です
 時間計算量 O(1)

-size ()->size_type
 要素数を返します
 時間計算量 O(1)

-at (size_type index)->const_reference
 index で指定した要素にアクセスします
 範囲外の場合 out_of_range 例外を送出します
 時間計算量 O(logN)

-operator[] (size_type index)->const_reference
 index で指定した要素にアクセスします
 時間計算量 O(logN)

-fold (size_type first, size_type last)->value_type
 [first, last) の和を返します
 first == last のとき 単位元を返します
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
 index で指定した要素を f を適用した値で更新します
 時間計算量 O(logN)

-update (size_type first, size_type last const operator_type &data)
 [first, last) に data を作用させます
 時間計算量 O(logN)


※N:全体の要素数
※各種関数の時間計算量を O(1) と仮定

*/
