#include <cassert>
#include <utility>
#include <vector>

template <class Band> class sparse_table {
public:
  using value_structure = Band;
  using value_type = typename value_structure::value_type;

private:
  using container_type = ::std::vector<value_type>;

public:
  using size_type = typename container_type::size_type;

private:
  static size_type msb(size_type c) {
#ifdef __has_builtin
    return 31 - __builtin_clz(c);
#else
    ::std::size_t ret = 0;
    if (c >> 16)
      c >>= 16, ret += 16;
    if (c >> 8)
      c >>= 8, ret += 8;
    if (c >> 4)
      c >>= 4, ret += 4;
    if (c >> 2)
      c >>= 2, ret += 2;
    return ret + (c >> 1);
#endif
  }
  ::std::vector<container_type> matrix;

public:
  sparse_table() : matrix() {}
  template <class InputIterator>
  sparse_table(InputIterator first, InputIterator last) : matrix() {
    matrix.emplace_back(first, last);
    const size_type size = matrix.front().size();
    for (size_type i = 1; i << 1 <= size; i <<= 1) {
      const size_type csz = size + 1 - (i << 1);
      container_type v;
      v.reserve(csz);
      for (size_type j = 0; j < csz; ++j)
        v.emplace_back(
            value_structure::operation(matrix.back()[j], matrix.back()[j + i]));
      matrix.emplace_back(::std::move(v));
    }
  }

  size_type size() const { return matrix.empty() ? 0 : matrix.front().size(); }
  bool empty() const { return size() == 0; }

  value_type fold(const size_type first, const size_type last) const {
    assert(first < last);
    assert(last <= size());
    const size_type t = msb(last - first);
    return value_structure::operation(
        matrix[t][first], matrix[t][last - (static_cast<size_type>(1) << t)]);
  }
};

/*

verify:http://judge.u-aizu.ac.jp/onlinejudge/review.jsp?rid=3095332#1

template<class Band>
class sparse_table;

sparse_table は半束の区間和を高速に計算するデータ構造です
空間計算量 O(NlogN)


テンプレートパラメータ
-class Band
 -結合律
  ∀a, ∀b, ∀c, a·(b·c) = (a·b)·c
 -冪等律
  ∀a, a·a = a
 以上の条件を満たす代数的構造 (冪等半群)

 以下のメンバを要求します
 -value_type
  台集合の型
 -static operation (value_type, value_type)->value_type
  2引数を取り、演算した結果を返す静的関数


メンバ型
-value_structure
 構造の型 (Band)

-value_type
 要素となる台集合の型 (value_structure::value_type)

-size_type
 符号なし整数型


メンバ関数
template<class InputIterator>
-(constructor) (InputIterator first, InputIterator last)
 [first, last) の要素から sparse_table を構築します
 時間計算量 O(NlogN)

-size ()->size_type
 要素数を返します
 時間計算量 O(1)

-empty ()->bool
 size()==0 と等価です
 時間計算量 O(1)

-fold (size_type first, size_type last)->value_type
 [first, last) の和を返します
 時間計算量 O(1)


※N:全体の要素数
※value_structure の各関数の時間計算量を O(1) と仮定

*/
