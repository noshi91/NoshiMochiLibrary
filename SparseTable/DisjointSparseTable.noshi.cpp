#include <cassert>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>

std::size_t bsr32(std::uint_fast32_t c) {
#ifdef __GNUC__
  return 31 ^ __builtin_clz(c);
#else
  std::size_t ret = 0;
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

template <class Semigroup> class DisjointSparseTable {
public:
  using container_type = std::vector<std::vector<Semigroup>>;
  using value_type = typename container_type::value_type::value_type;
  using const_reference = typename container_type::value_type::const_reference;
  using size_type = typename container_type::value_type::size_type;

protected:
  container_type c;

private:
  template <class TT = value_type,
            std::enable_if_t<std::is_default_constructible<TT>::value,
                             std::nullptr_t> = nullptr>
  value_type fold_(size_type first, size_type last) const {
    assert(first <= size());
    assert(last <= size());
    assert(first <= last);
    if (first == last)
      return value_type();
    if (first == --last)
      return c.front()[first];
    const size_type h = bsr32(first ^ last);
    return c[h][first] + c[h][last];
  }
  template <class TT = value_type,
            std::enable_if_t<!std::is_default_constructible<TT>::value,
                             std::nullptr_t> = nullptr>
  value_type fold_(size_type first, size_type last) const {
    assert(first < size());
    assert(last <= size());
    assert(first < last);
    if (first == --last)
      return c.front()[first];
    const size_type h = bsr32(first ^ last);
    return c[h][first] + c[h][last];
  }

public:
  DisjointSparseTable() : c() {}
  template <class InputIterator>
  DisjointSparseTable(InputIterator first, InputIterator last) : c() {
    c.emplace_back(first, last);
    const size_type s = c.front().size();
    if (!s)
      c.clear();
    else
      c.resize(s == static_cast<size_type>(1) ? 1 : (bsr32(s - 1) + 1));
    typename container_type::value_type temp;
    temp.reserve(s);
    for (size_type i = 1; i < c.size(); ++i) {
      const size_type width = static_cast<size_type>(1) << i;
      for (size_type j = width; j < s; j += width << 1) {
        temp.emplace_back(c.front()[j - 1]);
        for (size_type k = 2; k <= width; ++k)
          temp.emplace_back(c.front()[j - k] + temp.back());
        for (size_type k = width - 1; ~k; --k)
          c[i].emplace_back(std::move(temp[k]));
        temp.clear();
        c[i].emplace_back(c.front()[j]);
        for (size_type k = 1; k < width && j + k < s; ++k)
          c[i].emplace_back(c[i].back() + c.front()[j + k]);
      }
    }
  }
  DisjointSparseTable(const std::vector<value_type> &x)
      : DisjointSparseTable(x.cbegin(), x.cend()) {}

  size_type size() const { return c.empty() ? 0 : c.front().size(); }
  bool empty() const { return c.empty(); }

  const_reference operator[](const size_type index) const {
    assert(index < size());
    return c.front()[index];
  }
  value_type fold(const size_type first, const size_type last) const {
    return fold_(first, last);
  }
};

/*

verify:http://judge.u-aizu.ac.jp/onlinejudge/review.jsp?rid=2859015#1

template<class Semigroup>
class DisjointSparseTable;

DisjointSparseTableは静的な半群列の区間和を高速に計算するデータ構造です
空間計算量 O(NlogN)


テンプレートパラメータ
-typename Semigroup
 結合律 ∀a, ∀b, ∀c, a + (b + c) = (a + b) + c
 以上の条件を満たす代数的構造 (半群)

 -加法   :operator+(2項)
  以上のように定義されている必要があります


メンバ型
-container_type
 内部実装のコンテナ型

-value_type
 要素の型

-const_reference
 要素へのconst参照型

-size_type
 符号なし整数型


メンバ関数
template<class InputIterator>
-(constructor) (InputIterator first, InputIterator last)
 イテレータから構築します
 時間計算量 O(NlogN)

-(constructor) (const std::vector<value_type> &x)
 コンテナを受け取り構築します
 時間計算量 O(NlogN)

-size ()->size_type
 要素数を返します
 時間計算量 O(1)

-empty ()->bool
 要素が空かどうか判定します
 時間計算量 O(1)

-operator[] (size_type index)->const_reference
 index で指定した要素にアクセスします
 時間計算量 O(1)

-fold (size_type first, size_type last)->value_type
 [first, last) の和を返します
 -first == last となる呼び出しについて、
  1.value_type がデフォルト構築可能な場合
    value_type() を返します
  2.value_type がデフォルト構築可能でない場合
    挙動は保証されません (assertにより終了します)
 時間計算量 O(1) ※1


※N:全体の要素数
※operator+ の時間計算量を O(1) と仮定
※1 bsr32() の時間計算量を O(1) と仮定 - bit演算の場合 O(loglogN)

*/