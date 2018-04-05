#include <cassert>
#include <functional>
#include <utility>
#include <vector>

template <typename ValueMonoid, typename OperatorMonoid, class Modify>
class LazySegmentTree {
public:
  using value_type = ValueMonoid;
  using reference = value_type &;
  using const_reference = const value_type &;
  using operator_type = OperatorMonoid;

private:
  using container_type = std::vector<std::pair<value_type, operator_type>>;

public:
  using size_type = typename container_type::size_type;

private:
  const Modify m;
  const size_type size_, height, capacity;
  container_type tree;
  static size_type getheight(const size_type &size) noexcept {
    size_type ret = 0;
    while (static_cast<size_type>(1) << ret < size)
      ++ret;
    return ret;
  }
  value_type reflect(const size_type &index) {
    return m(tree[index].first, tree[index].second);
  }
  void recalc(const size_type &index) {
    tree[index].first = reflect(index << 1) + reflect(index << 1 | 1);
  }
  void assign(const size_type &index, const operator_type &data) {
    tree[index].second = tree[index].second + data;
  }
  void push(const size_type &index) {
    assign(index << 1, tree[index].second);
    assign(index << 1 | 1, tree[index].second);
    tree[index].second = operator_type();
  }
  void propagate(const size_type &index) {
    for (size_type i = height; i; --i)
      push(index >> i);
  }
  void thrust(const size_type &index) {
    tree[index].first = reflect(index);
    push(index);
  }
  void evaluate(const size_type &index) {
    for (size_type i = height; i; --i)
      thrust(index >> i);
  }
  void build(size_type index) {
    while (index >>= 1)
      recalc(index);
  }

public:
  explicit LazySegmentTree(const size_type &size, const Modify &m = Modify())
      : m(m), size_(size), height(getheight(size_)),
        capacity(static_cast<size_type>(1) << height), tree(capacity << 1) {}
  void update(size_type begin, size_type end, const operator_type &data) {
    assert(begin <= end);
    assert(begin <= size());
    assert(end <= size());
    begin += capacity;
    end += capacity;
    propagate(begin);
    propagate(end - 1);
    for (size_type left = begin, right = end; left < right;
         left >>= 1, right >>= 1) {
      if (left & 1)
        assign(left++, data);
      if (right & 1)
        assign(right - 1, data);
    }
    build(begin);
    build(end - 1);
  }
  void update(size_type index,
              const std::function<value_type(const_reference)> &f) {
    assert(index < size());
    index += capacity;
    propagate(index);
    tree[index].first = f(reflect(index));
    tree[index].second = operator_type();
    build(index);
  }
  void update(const size_type index, const_reference data) {
    assert(index < size());
    update(index, [&data](const_reference d) { return data; });
  }
  value_type range(size_type begin, size_type end) {
    assert(begin <= end);
    assert(begin <= size());
    assert(end <= size());
    begin += capacity;
    end += capacity;
    evaluate(begin);
    evaluate(end - 1);
    value_type retL, retR;
    for (; begin < end; begin >>= 1, end >>= 1) {
      if (begin & 1)
        retL = retL + reflect(begin++);
      if (end & 1)
        retR = reflect(end - 1) + retR;
    }
    return retL + retR;
  }
  size_type search(const std::function<bool(const_reference)> &b) {
    if (b(value_type()))
      return 0;
    if (!b(reflect(1)))
      return size() + 1;
    value_type acc;
    size_type i = 1;
    while (i < capacity) {
      thrust(i);
      if (!b(acc + reflect(i <<= 1)))
        acc = acc + reflect(i++);
    }
    return i - capacity + 1;
  }
  const_reference operator[](size_type index) {
    assert(index < size());
    index += capacity;
    evaluate(index);
    tree[index].first = reflect(index);
    tree[index].second = operator_type();
    return tree[index].first;
  }
  size_type size() const noexcept { return size_; }
  bool empty() const noexcept { return !size_; }
};

template <typename V, typename O, class F>
LazySegmentTree<V, O, F>
make_Lazy(const typename LazySegmentTree<V, O, F>::size_type &size,
          const F &f) {
  return LazySegmentTree<V, O, F>(size, f);
}

/*

verify:http://judge.u-aizu.ac.jp/onlinejudge/review.jsp?rid=2762794#1
      :http://judge.u-aizu.ac.jp/onlinejudge/review.jsp?rid=2762798#1
      :http://judge.u-aizu.ac.jp/onlinejudge/review.jsp?rid=2762820#1

template<typename ValueMonoid, typename OperatorMonoid, class Modify>
class LazySegmentTree;

LazySegmentTreeはモノイドの区間和と区間更新を高速に計算するデータ構造です
空間計算量 O(N)


テンプレートパラメータ
-typename ValueMonoid
 結合律 ∀a, ∀b, ∀c, a + (b + c) = (a + b) + c
 単位元 ∃e, ∀a, e + a = a + e = a
 以上の条件を満たす代数的構造 (モノイド)

 -加法   :operator+(2項)
 -単位元 :デフォルトコンストラクタ
  以上のように定義されている必要があります

-typename OperatorMonoid
 結合律 ∀a, ∀b, ∀c, a + (b + c) = (a + b) + c
 単位元 ∃e, ∀a, e + a = a + e = a
 以上の条件を満たす代数的構造 (モノイド)

 -加法   :operator+(2項)
 -単位元 :デフォルトコンストラクタ
  以上のように定義されている必要があります

-class Modify
 ValueMonoid に OperatorMonoid を作用させた結果を返す、
 operator() が定義されたクラスか関数ポインタ


 また、以下の条件が成立する必要があります
 (ValueMonoid を V、OperatorMonoid を O、Modifyを m と表記)
 閉性     ∀a∈V, ∀b∈O, m(a, b)∈V
 単位元   ∃e∈O, ∀a∈V, m(a, e) = a
 結合律   ∀a∈V, ∀b∈O, ∀c∈O, m(a, (b + c)) = m(m(a, b), c)
 分配法則 ∀a∈V, ∀b∈V, ∀c∈O, m(a, c) + m(b, c) = m((a + b), c)


メンバ型
-value_type
 要素の型 (ValueMonoid)

-reference
 要素(value_type)への参照型 (value_type &)

-const_reference
 要素(value_type)へのconst参照型 (const value_type &)

-operator_type
 演算要素の型 (OperatorMonoid)

-size_type
 符号なし整数型 (std::vector<std::pair<value_type, operator_type>>::size_type)


メンバ関数
-(constructor) (size_type size,Modify m = Modify())
 関数オブジェクトを受け取り、要素数 size の LazySegmentTree を構築します
 各要素は単位元で初期化されます
 時間計算量 O(N)

-update (size_type begin, size_type end const operator_type &data)
 [begin, end) に data を作用させます
 時間計算量 O(logN)

-update (size_type index, std::function<value_type(const_reference)> f)
 index で指定した要素を f を適用した値で更新します
 時間計算量 O(logN)

-update (size_type index, const_reference data)
 index で指定した要素を data に更新します
 時間計算量 O(logN)

-range (size_type begin, size_type end)->value_type
 [begin, end) の和を返します
 begin == end のとき 単位元を返します
 時間計算量 O(logN)

-search (std::function<bool(const_reference)> b)->size_type
 b(range(0, i - 1)) が false を返し、
 b(range(0, i))     が true  を返すような i を返します
 b(range(0, -1))         は false、
 b(range(0, size() + 1)) は true と扱います
 時間計算量 O(logN)

-operator[] (size_type index)->const_reference
 index で指定した要素にアクセスします
 時間計算量 O(logN)

-size ()->size_type
 要素数を返します
 時間計算量 O(1)

-empty ()->bool
 size()==0 と同値です
 時間計算量 O(1)


非メンバ関数
template<typename V, typename O, class F>
-make_Lazy (size_type size, F f)->LazySegmentTree<V, O, F>
 関数オブジェクトを受け取り LazySegmentTree を構築するヘルパ関数


※N:全体の要素数
※f() の時間計算量を O(1) と仮定

*/