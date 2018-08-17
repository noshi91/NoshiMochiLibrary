#include <cassert>
#include <utility>
#include <vector>

template <class Semigroup> class disjoint_sparse_table {
public:
  using value_structure = Semigroup;
  using value_type = typename value_structure::value_type;

private:
  using container_type = ::std::vector<::std::vector<value_type>>;

public:
  using const_reference = typename container_type::value_type::const_reference;
  using size_type = typename container_type::value_type::size_type;

protected:
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

  container_type table;

public:
  disjoint_sparse_table() : table() {}
  template <class InputIterator>
  disjoint_sparse_table(InputIterator first, InputIterator last) : table() {
    table.emplace_back(first, last);
    const size_type size = table.front().size();
    for (size_type i = 2; i < size; i <<= 1) {
      typename container_type::value_type v;
      v.reserve(size);
      for (size_type j = i; j < size; j += i << 1) {
        v.emplace_back(table.front()[j - 1]);
        for (size_type k = 2; k <= i; ++k)
          v.emplace_back(
              value_structure::operation(table.front()[j - k], v.back()));
        v.emplace_back(table.front()[j]);
        for (size_type k = 1; k < i && j + k < size; ++k)
          v.emplace_back(
              value_structure::operation(v.back(), table.front()[j + k]));
      }
      table.emplace_back(::std::move(v));
    }
  }

  size_type size() const { return table.empty() ? 0 : table.front().size(); }
  bool empty() const { return size() == 0; }

  value_type fold_closed(const size_type first, const size_type last) const {
    assert(first <= last);
    assert(last < size());
    if (first == last) {
      return table.front()[first];
    } else {
      const size_type p = msb(first ^ last);
      return value_structure::operation(
          table[p][first ^ (static_cast<size_type>(1) << p) - 1],
          table[p][last]);
    }
  }
  const_reference operator[](const size_type index) const {
    assert(index < size());
    return table.front()[index];
  }
};

/*

verify:http://judge.u-aizu.ac.jp/onlinejudge/review.jsp?rid=3095436#1

template<class Semigroup>
class disjoint_sparse_table;

disjoint_sparse_table は静的な半群列の区間和を高速に計算するデータ構造です
空間計算量 O(NlogN)


テンプレートパラメータ
-class Semigroup
 -結合律
  ∀a, ∀b, ∀c, a·(b·c) = (a·b)·c
 以上の条件を満たす代数的構造 (半群)

 以下のメンバを要求します
 -value_type
  台集合の型
 -static operation (value_type, value_type)->value_type
  2引数を取り、演算した結果を返す静的関数


メンバ型
-value_structure
 構造の型 (Seemigroup)

-value_type
 要素となる台集合の型 (value_structure::value_type)

-const_reference
 要素へのconst参照型

-size_type
 符号なし整数型


メンバ関数
template<class InputIterator>
-(constructor) (InputIterator first, InputIterator last)
 [first, last) の要素から disjoint_sparse_table を構築します
 時間計算量 O(NlogN)

-empty ()->bool
 要素が空かどうか判定します
 時間計算量 O(1)

-size ()->size_type
 要素数を返します
 時間計算量 O(1)

-fold (size_type first, size_type last)->value_type
 [first, last) の和を返します

-fold (size_type first, size_type last)->value_type
 [first, last) の和を返します
 時間計算量 O(1)

-operator[] (size_type index)->const_reference
 index で指定した要素にアクセスします
 時間計算量 O(1)


※ N:全体の要素数
※ value_structure::operation() の時間計算量を O(1) と仮定
※ msb() の時間計算量を O(1) と仮定 - デフォルトの実装の場合 O(loglogN)

*/
