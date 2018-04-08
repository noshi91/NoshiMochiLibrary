#include <cassert>
#include <vector>

template <typename SemiLattice> class SparseTable {
public:
  using value_type = SemiLattice;
  using container_type = std::vector<value_type>;
  using size_type = typename container_type::size_type;

private:
  static size_type clz32(size_type c) {
#ifdef __GNUC__
    return 31 ^ __builtin_clz(c);
#else
    size_type ret = 0;
    if (c & 0xFFFF0000)
      c &= 0xFFFF0000, ret |= 16;
    if (c & 0xFF00FF00)
      c &= 0xFF00FF00, ret |= 8;
    if (c & 0xF0F0F0F0)
      c &= 0xF0F0F0F0, ret |= 4;
    if (c & 0xCCCCCCCC)
      c &= 0xCCCCCCCC, ret |= 2;
    if (c & 0xAAAAAAAA)
      ret |= 1;
    return ret;
#endif
  }
  std::vector<container_type> matrix;

public:
  SparseTable(const container_type &a)
      : matrix(a.empty() ? 0 : clz32(a.size()) + 1) {
    if (!a.empty())
      matrix[0].reserve(a.size());
    for (size_type i = 0; i < a.size(); ++i)
      matrix[0].emplace_back(a[i]);
    for (size_type i = 1; i < matrix.size(); ++i) {
      const size_type msiz = a.size() + 1 - (static_cast<size_type>(1) << i);
      matrix[i].reserve(msiz);
      for (size_type j = 0; j < msiz; ++j)
        matrix[i].emplace_back(
            matrix[i - 1][j] +
            matrix[i - 1][j + (static_cast<size_type>(1) << (i - 1))]);
    }
  }
  value_type range(const size_type begin, const size_type end) const {
    assert(begin < end);
    assert(begin < size());
    assert(end <= size());
    const size_type t = clz32(end - begin);
    return matrix[t][begin] + matrix[t][end - (static_cast<size_type>(1) << t)];
  }
  size_type size() const noexcept { return empty() ? 0 : matrix[0].size(); }
  bool empty() const noexcept { return matrix.empty(); }
};

/*

verify:http://judge.u-aizu.ac.jp/onlinejudge/review.jsp?rid=2766053#1

template<typename SemiLattice>
class SparseTable;

SparseTable は半束の区間和を高速に計算するデータ構造です
空間計算量 O(NlogN)


テンプレートパラメータ
-typename SemiLattice
 結合律 ∀a, ∀b, ∀c, a + (b + c) = (a + b) + c
 可換律 ∀a, ∀b, a + b = b + a
 冪等律 ∀a, a + a = a
 以上の条件を満たす代数的構造 (半束)

 -加法   :operator+(2項)
  以上のように定義されている必要があります


メンバ型
-value_type
 要素の型 (SemiLattice)

-container_type
 コンテナの型 (std::vector<value_type>)

-size_type
 符号なし整数型 (container_type::size_type)


メンバ関数
-(constructor) (std::vector<value_type> a)
 a の各要素を元に SparseTable を構築します
 時間計算量 O(NlogN)

-range (size_type begin, size_type end)->value_type
 [begin, end) の和を返します
 begin>=end の際の動作は保証されません
 時間計算量 O(loglogN)

-size ()->size_type
 要素数を返します
 時間計算量 O(1)

-empty ()->bool
 size()==0 と同値です
 時間計算量 O(1)


※N:全体の要素数
※operator+ の時間計算量を O(1) と仮定

*/