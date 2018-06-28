#include <cassert>
#include <functional>
#include <utility>
#include <vector>

template <class T, class Container = ::std::vector<T>,
          class Compare = ::std::less_equal<typename Container::value_type>>
class interval_heap {

public:
  using container_type = Container;
  using value_compare = Compare;
  using value_type = typename container_type::value_type;
  using reference = typename container_type::reference;
  using const_reference = typename container_type::const_reference;
  using size_type = typename container_type::size_type;

protected:
  container_type c;
  value_compare comp;

private:
  static size_type left(const size_type i) { return (i - 1) << 1; }
  static size_type right(const size_type i) { return (i << 1) - 1; }
  bool normalize(const size_type l, const size_type g) {
    return comp(c[l], c[g]) ? false : (::std::swap(c[l], c[g]), true);
  }
  void build() {
    const size_type s = (size() + 1) >> 1;
    if (!(size() & 1))
      if (normalize(left(s), right(s)))
        buildmin(s);
      else
        buildmax(s);
    else if (size() != 1)
      if (normalize(left(s), right(s >> 1)))
        buildmax(s >> 1);
      else
        buildmin(s);
  }
  void buildmin(size_type i) {
    while (i != 1 && normalize(left(i >> 1), left(i)))
      i >>= 1;
  }
  void buildmax(size_type i) {
    while (i != 1 && normalize(right(i), right(i >> 1)))
      i >>= 1;
  }

public:
  interval_heap() : c(), comp() {}
  explicit interval_heap(const value_compare &x) : c(), comp(x) {}
  explicit interval_heap(const value_compare &x, const container_type &other)
      : c(other), comp(x) {}
  explicit interval_heap(const value_compare &x, container_type &&other)
      : c(::std::move(other)), comp(x) {}

  bool empty() const { return c.empty(); }
  size_type size() const { return c.size(); }

  const_reference min() const {
    assert(!empty());
    return c[0];
  }
  const_reference max() const {
    assert(!empty());
    return size() == 1 ? c[0] : c[1];
  }

  void push(const value_type &x) {
    c.push_back(x);
    build();
  }
  void push(value_type &&x) {
    c.push_back(::std::move(x));
    build();
  }
  template <class... Args> void emplace(Args &&... args) {
    c.emplace_back(::std::forward<Args>(args)...);
    build();
  }
  void pop_min() {
    assert(!empty());
    ::std::swap(c.front(), c.back());
    c.pop_back();
    const size_type s = size();
    size_type i = 2;
    while (left(i) < s) {
      if (left(i | 1) < s && comp(c[left(i | 1)], c[left(i)]))
        i |= 1;
      if (!normalize(left(i >> 1), left(i)))
        return;
      if (right(i) < s)
        normalize(left(i), right(i));
      i <<= 1;
    }
  }
  void pop_max() {
    assert(!empty());
    if (size() == 1) {
      c.pop_back();
      return;
    }
    ::std::swap(c[1], c.back());
    c.pop_back();
    const size_type s = size();
    size_type i = 2;
    while (right(i) < s) {
      if (right(i | 1) < s && comp(c[right(i)], c[right(i | 1)]))
        i |= 1;
      if (!normalize(right(i), right(i >> 1)))
        return;
      normalize(left(i), right(i));
      i <<= 1;
    }
  }
};

/*

verify:https://beta.atcoder.jp/contests/arc098/submissions/2751630
      :https://beta.atcoder.jp/contests/abc035/submissions/2751632

template<class T, class Container = ::std::vector<T>,
         class Compare = ::std::less<typename Container::value_type>>
class interval_heap;

interval_heapは両端優先度付きキュー(DEPQ)の一種であり、
最大値と最小値双方の管理をするデータ構造です
空間計算量 O(N)


テンプレートパラメータ
-class T
 要素の型

-class Container
 内部実装のコンテナクラス
 デフォルトでは ::std::vector<T> が使用されます

-class Compare
 Tが全順序集合を成すように大小比較を行う比較クラス
 デフォルトでは ::std::less_equal<T> が使用されます


メンバ型
-container_type
 コンテナクラス (Container)

-value_compare
 比較クラス (Compare)

-vaue_type
 要素の型 (Container::value_type)

-reference
 要素への参照型 (Container::reference)

-const_reference
 要素へのconst参照型 (Container::const_reference)

-size_type
 符号なし整数型 (Container::size_type)


メンバ関数
-(constructor) ()
 要素を持たない状態で interval_heap を構築します
 第一引数は比較関数、第二引数はコンテナの初期化に使用されます
 時間計算量 O(1)

-empty ()->bool
 要素が空かどうかを判定します
 時間計算量 O(1)

-size ()->size_type
 要素数を取得します
 時間計算量 O(1)

-min ()->const_reference
 最小値を取得します
 時間計算量 O(1)

-max ()->const_reference
 最大値を取得します
 時間計算量 O(1)

-push (const value_type &x)
 x を要素として追加します
 時間計算量 O(logN) ※1

-template<class... Args>
 emplace (Args&&... args)
 コンストラクタの引数から直接構築で要素を追加します
 時間計算量 O(logN) ※1

-pop_min ()
 最小値を削除します
 時間計算量 O(logN) ※1

-pop_max ()
 最大値を削除します
 時間計算量 O(logN) ※1


※N:要素数
※比較関数の時間計算量を O(1) と仮定
※1 各関数内ではコンテナの以下の関数を1回呼び出している
    push():push_back()
    emplace():emplace_back()
    pop_min()/pop_max():pop_back()
    よって、計算量はそれに従う

*/
