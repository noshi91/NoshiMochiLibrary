#include <cassert>
#include <functional>
#include <vector>

template <typename Monoid> class SegmentTree {
public:
  using value_type = Monoid;
  using reference = value_type &;
  using const_reference = const value_type &;
  using container_type = std::vector<value_type>;
  using size_type = typename container_type::size_type;

private:
  const size_type size_, capacity;
  container_type tree;
  static size_type getsize(const size_type &size) noexcept {
    size_type ret = 1;
    while (ret < size)
      ret <<= 1;
    return ret;
  }
  void recalc(const size_type &index) {
    tree[index] = tree[index << 1] + tree[index << 1 | 1];
  }

public:
  explicit SegmentTree(const size_type &size)
      : size_(size), capacity(getsize(size_)), tree(capacity << 1) {}
  void update(size_type index,
              const std::function<value_type(const_reference)> &f) {
    assert(index < size());
    index += capacity;
    tree[index] = f(tree[index]);
    while (index >>= 1)
      recalc(index);
  }
  void update(const size_type &index, const_reference data) {
    update(index, [&data](const_reference x) { return data; });
  }
  value_type range(size_type begin, size_type end) const {
    assert(begin <= end);
    assert(begin <= size());
    assert(end <= size());
    value_type retL, retR;
    for (begin += capacity, end += capacity; begin < end;
         begin >>= 1, end >>= 1) {
      if (begin & 1)
        retL = retL + tree[begin++];
      if (end & 1)
        retR = tree[end - 1] + retR;
    }
    return retL + retR;
  }
  size_type search(const std::function<bool(const_reference)> &b) const {
    if (b(value_type()))
      return 0;
    if (!b(tree[1]))
      return size() + 1;
    value_type acc;
    size_type i = 1;
    while (i < capacity)
      if (!b(acc + tree[i <<= 1]))
        acc = acc + tree[i++];
    return i - capacity + 1;
  }
  const_reference operator[](const size_type &index) const {
    assert(index < size());
    return tree[index + capacity];
  }
  size_type size() const noexcept { return size_; }
  bool empty() const noexcept { return !size_; }
};

/*

verify:https://beta.atcoder.jp/contests/arc033/submissions/2304097
      :http://judge.u-aizu.ac.jp/onlinejudge/review.jsp?rid=2762580#1
      :http://judge.u-aizu.ac.jp/onlinejudge/review.jsp?rid=2762583#1

template<typename Monoid>
class SegmentTree;

SegmentTreeはモノイドの区間和を高速に計算するデータ構造です
空間計算量 O(N)


テンプレートパラメータ
-typename Monoid
 結合律 ∀a, ∀b, ∀c, a + (b + c) = (a + b) + c
 単位元 ∃e, ∀a, e + a = a + e = a
 以上の条件を満たす代数的構造 (モノイド)

 -加法   :operator+(2項)
 -単位元 :デフォルトコンストラクタ
  以上のように定義されている必要があります


メンバ型
-value_type
 要素の型 (Monoid)

-reference
 要素(value_type)への参照型 (value_type &)

-const_reference
 要素(value_type)へのconst参照型 (const value_type &)

-container_type
 コンテナの型 (std::vector<value_type>)

-size_type
 符号なし整数型 (container_type::size_type)


メンバ関数
-(constructor) (size_type size)
 要素数 size の SegmentTree を構築します
 各要素は単位元で初期化されます
 時間計算量 O(N)

-update (size_type index, const_reference data)
 index で指定した要素を data に更新します
 時間計算量 O(logN)

-update (size_type index, std::function<value_type(const_reference)> f)
 index で指定した要素を f を適用した値で更新します
 時間計算量 O(logN)

-range (size_type begin, size_type end)->value_type
 [begin, end)の和を返します
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
 時間計算量 O(1)

-size ()->size_type
 要素数を返します
 時間計算量 O(1)

-empty ()->bool
 size()==0 と同値です
 時間計算量 O(1)


※N:全体の要素数
※operator+ の時間計算量を O(1) と仮定

*/