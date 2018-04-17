#include <cassert>
#include <functional>
#include <utility>
#include <vector>

template <typename T, class Container = std::vector<T>,
          class Compare = std::less<typename Container::value_type>>
class IntervalHeap {

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
  static constexpr size_type z = ~static_cast<size_type>(1);
  bool normalize(const size_type x, const size_type y) {
    if (!comp(c[y], c[x]))
      return false;
    std::swap(c[x], c[y]);
    return true;
  }
  void build() {
    const size_type s = size() + 1;
    if (s & 1) {
      normalize(s & z, s);
      buildmin(s >> 1);
      buildmax(s >> 1);
      return;
    }
    if (s == 2)
      return;
    if (normalize(s, s >> 1 | 1))
      buildmax(s >> 2);
    buildmin(s >> 1);
  }
  void buildmin(size_type i) {
    while (i > 1 && normalize(i & z, i << 1))
      i >>= 1;
  }
  void buildmax(size_type i) {
    while (i > 1 && normalize(i << 1 | 1, i | 1))
      i >>= 1;
  }

public:
  explicit IntervalHeap(const value_compare &x = value_compare())
      : c(2), comp(x) {}
  bool empty() const { return c.size() == 2; }
  size_type size() const { return c.size() - 2; }
  const_reference min() const {
    assert(!empty());
    return c[2];
  }
  const_reference max() const {
    assert(!empty());
    return size() == 1 ? c[2] : c[3];
  }
  void push(const value_type &x) {
    c.push_back(x);
    build();
  }
  void push(value_type &&x) {
    c.push_back(std::move(x));
    build();
  }
  template <class... Args> void emplace(Args &&... args) {
    c.emplace_back(std::forward<Args>(args)...);
    build();
  }
  void pop_min() {
    assert(!empty());
    const size_type s = size() + 1;
    std::swap(c[2], c[s]);
    c.pop_back();
    size_type i = 4;
    while (i < s) {
      if ((i | 2) < s && !comp(c[i], c[i | 2]))
        i |= 2;
      if (!normalize(i >> 1 & z, i))
        return;
      if ((i | 1) < s)
        normalize(i, i | 1);
      i <<= 1;
    }
  }
  void pop_max() {
    assert(!empty());
    const size_type s = size() + 1;
    if (s == 2)
      return pop_min();
    std::swap(c[3], c[s]);
    c.pop_back();
    size_type i = 5;
    while (i < s) {
      if ((i | 2) < s && !comp(c[i | 2], c[i]))
        i |= 2;
      if (!normalize(i, i >> 1 | 1))
        return;
      normalize(i & z, i);
      i = (i & z) << 1 | 1;
    }
  }
};

/*

verify:https://beta.atcoder.jp/contests/abs/submissions/2372683
      :https://beta.atcoder.jp/contests/abc035/submissions/2372707

template<typename T, class Container = std::vector<T>,
         class Compare = std::less<typename Container::value_type>>
class IntervalHeap;

IntervalHeapは両端優先度付きキュー(DEPQ)の一種であり、
最大値と最小値双方の管理をするデータ構造です
空間計算量 O(N)


テンプレートパラメータ
-typename T
 要素の型

-class Container
 内部実装のコンテナクラス
 デフォルトでは std::vector が使用されます

-class Compare
 比較クラス
 デフォルトでは std::less が使用されます


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
-(constructor) (const Compare &x = Compare())
 x を比較用のオブジェクトとして、
 要素を持たない状態で IntervalHeap を構築します

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
 時間計算量 償却O(logN) ※1

-template<class... Args>
 emplace (Args&&... args)
 コンストラクタの引数から直接構築で要素を追加します
 時間計算量 償却O(logN) ※1

-pop_min ()
 最小値を削除します
 時間計算量 償却O(logN) ※1

-pop_max ()
 最大値を削除します
 時間計算量 償却O(logN) ※1


※N:要素数
※比較の時間計算量を O(1) と仮定
※1 償却計算量はコンテナクラスの push_back()/emplace_back()/pop_back()
    に起因している

*/