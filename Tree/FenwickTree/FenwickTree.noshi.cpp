#include <cassert>
#include <functional>
#include <vector>

template <typename CommutativeMonoid,
          class Container = std::vector<CommutativeMonoid>>
class FenwickTree {

public:
  using value_type = typename Container::value_type;
  using reference = typename Container::reference;
  using const_reference = typename Container::const_reference;
  using size_type = typename Container::size_type;
  using container_type = Container;

private:
  const size_type size_, capacity;
  container_type tree;
  static size_type getsize(const size_type siz) noexcept {
    size_type ret = 1;
    while (ret < siz)
      ret <<= 1;
    return ret;
  }

public:
  explicit FenwickTree(const size_type size)
      : size_(size), capacity(getsize(size_)), tree(capacity + 1) {}
  void update(size_type index, const_reference diff) {
    assert(index < size());
    for (++index; index <= capacity; index += index & ~index + 1)
      tree[index] = tree[index] + diff;
  }
  value_type range(size_type end) const {
    assert(end <= size());
    value_type ret;
    for (; end; end &= end - 1)
      ret = tree[end] + ret;
    return ret;
  }
  size_type search(const std::function<bool(const_reference)> &b) const {
    if (!b(tree.back()))
      return size_;
    size_type i = 0, k = capacity;
    value_type acc;
    while (k >>= 1)
      if (!b(acc + tree[i + k]))
        acc = acc + tree[i += k];
    return i;
  }
  size_type size() const noexcept { return size_; }
  bool empty() const noexcept { return !size_; }
};

/*

verify:https://beta.atcoder.jp/contests/arc033/submissions/2298393

template<typename CommutativeMonoid,
         class Container = std::vector<CommutativeMonoid>>
class FenwickTree;

FenwickTreeは可換モノイドの区間和を高速に計算するデータ構造です
空間計算量 O(N)


テンプレートパラメータ
-typename CommutativeMonoid
 結合律 a + (b + c) = (a + b) + c
 交換律 a + b = b + a
 単位元 ∃e [∀a [e + a = a]]
 以上の条件を満たす代数的構造 (可換モノイド)

 -加法   :operator+(2項)
 -単位元 :デフォルトコンストラクタ
  以上のように定義されている必要があります

-class Container
 内部で使用するコンテナ型
 デフォルトでは std::vector<CommutativeMonoid> が使用されます


メンバ型
-value_type
 要素の型 (Container::value_type)

-reference
 要素(value_type)への参照型 (Container::reference)

-const_reference
 要素(value_type)へのconst参照型 (Container::const_reference)

-size_type
 符号なし整数型 (Container::size_type)


メンバ関数
-(constructor) (size_type size)
 size 個の要素からなる FenwickTree を構築します
 各要素は単位元で初期化されます
 時間計算量 O(N)

-update (size_type index, const_reference diff)
 index で指定した要素に diff を加算します
 時間計算量 O(logN)

-range (size_type end)-value_type
 [0, end)の和を返します
 end == 0 のとき 単位元を返します
 時間計算量 O(logN)

-search (std::function<bool(const_reference)> b)->size_type
 b(range(i + 1)) が true を返すような i のうち最小の値を返します
 そのような i が存在しない場合 N 以上の値を返します
 b(range(1   ~ i)) が false かつ
 b(range(i+1 ~ N)) が true  である必要があります
 時間計算量 O(logN)

-size ()->size_type
 要素数を返します
 時間計算量 O(1)

-empty ()->bool
 size()==0 と等価です
 時間計算量 O(1)


※N:全体の要素数
※operator+の時間計算量をO(1)と仮定

*/