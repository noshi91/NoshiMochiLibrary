#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

#ifdef __has_builtin
::std::size_t popcount64(::std::uint_fast64_t c) {
  return __builtin_popcountll(c);
}
#else
::std::size_t popcount64(::std::uint_fast64_t c) {
  c = (c & 0x5555555555555555) + ((c & 0xAAAAAAAAAAAAAAAA) >> 1);
  c = (c & 0x3333333333333333) + ((c & 0xCCCCCCCCCCCCCCCC) >> 2);
  return static_cast<::std::size_t>(
      ((c + (c >> 4)) & 0x0F0F0F0F0F0F0F0F) * 0x0101010101010101 >> 56 & 0x7f);
}
#endif

template <typename Integral,
          ::std::size_t bitsize = ::std::numeric_limits<Integral>::digits()>
class wavelet_matrix {
public:
  using value_type = Integral;
  using size_type = ::std::size_t;

private:
  using uint64 = ::std::uint_fast64_t;
  struct bitvector {
    ::std::vector<::std::pair<uint64, size_type>> dic;
    size_type cnt;
    value_type bit;
    bitvector() {}
    bitvector(const size_type bsize) : dic(bsize, {0, 0}) {}
    void set(const size_type index) {
      dic[index >> 6].first |= static_cast<uint64>(1) << (index & 0x3f);
    }
    void build() {
      const size_type len = dic.size();
      for (size_type i = 1; i < len; ++i)
        dic[i].second = dic[i - 1].second + popcount64(dic[i - 1].first);
    }
    size_type rank(const size_type last) const {
      return dic[last >> 6].second +
             popcount64(dic[last >> 6].first &
                        (static_cast<uint64>(1) << (last & 0x3f)) - 1);
    }
    bool access(const size_type index) const {
      return dic[index >> 6].first >> (index & 0x3f) & 1;
    }
  };
  ::std::array<bitvector, bitsize> matrix_;
  value_type not_found_;
  size_type size_;

public:
  wavelet_matrix() : matrix_(), size_(0), not_found_(0) {}
  template <class InputIter>
  wavelet_matrix(InputIter first, InputIter last,
                 const value_type not_found = 0)
      : size_(0), not_found_(not_found) {
    ::std::vector<value_type> vec0(first, last);
    size_ = vec0.size();
    const size_type len = size_, block = (len >> 5) + 1;
    ::std::vector<value_type> vec_l(len), vec_r(len);
    size_type l, r;
    value_type temp = static_cast<value_type>(1) << (bitsize - 1);
    for (auto &v : matrix_) {
      v = bitvector(block);
      v.bit = temp;
      temp = temp >> 1 & ~temp;
      l = 0;
      r = 0;
      for (size_type j = 0; j < len; ++j)
        if (vec0[j] & v.bit)
          vec_r[r++] = vec0[j], v.set(j);
        else
          vec_l[l++] = vec0[j];
      v.cnt = l;
      v.build();
      ::std::swap(vec0, vec_l);
      ::std::copy(vec_r.begin(), vec_r.begin() + r, vec0.begin() + l);
    }
  }

  size_type size() const noexcept { return size_; }
  bool empty() const noexcept { return size() == 0; }

  value_type access(size_type index) const {
    assert(index < size());
    value_type ret = 0;
    for (const auto &v : matrix_)
      if (v.access(index))
        ret |= v.bit, index = v.rank(index) + v.cnt;
      else
        index -= v.rank(index);
    return ret;
  }
  value_type operator[](const size_type index) const {
    assert(index < size());
    return access(index);
  }
  size_type rank(size_type first, size_type last, const value_type x) const {
    assert(first <= size());
    assert(last <= size());
    assert(first <= last);
    for (const auto &v : matrix_)
      if (x & v.bit)
        first = v.rank(first) + v.cnt, last = v.rank(last) + v.cnt;
      else
        first -= v.rank(first), last -= v.rank(last);
    return last - first;
  }
  value_type quantile(size_type first, size_type last, size_type k) const {
    assert(first < size());
    assert(last <= size());
    assert(first < last);
    assert(last - first > k);
    value_type ret = 0;
    for (const auto &v : matrix_) {
      const size_type l = v.rank(first), r = v.rank(last);
      if (r - l > k)
        first = l + v.cnt, last = r + v.cnt, ret |= v.bit;
      else
        first -= l, last -= r, k -= r - l;
    }
    return ret;
  }
  value_type rquantile(size_type first, size_type last, size_type k) const {
    assert(first < size());
    assert(last <= size());
    assert(first < last);
    assert(last - first > k);
    return quantile(first, last, last - first - k - 1);
  }
  size_type less_than(size_type first, size_type last,
                      const value_type x) const {
    assert(first <= size());
    assert(last <= size());
    assert(first <= last);
    size_type ret = 0;
    for (const auto &v : matrix_) {
      if (x & v.bit) {
        ret += last - first + v.rank(first) - v.rank(last);
        first = v.rank(first) + v.cnt;
        last = v.rank(last) + v.cnt;
      } else {
        first -= v.rank(first);
        last -= v.rank(last);
      }
    }
    return ret;
  }
  size_type at_least(size_type first, size_type last,
                     const value_type x) const {
    assert(first <= size());
    assert(last <= size());
    assert(first <= last);
    size_type ret = 0;
    for (const auto &v : matrix_) {
      if (x & v.bit) {
        first = v.rank(first) + v.cnt;
        last = v.rank(last) + v.cnt;
      } else {
        ret += v.rank(last) - v.rank(first);
        first -= v.rank(first);
        last -= v.rank(last);
      }
    }
    return ret + (last - first);
  }
  size_type greater_than(size_type first, size_type last,
                         const value_type x) const {
    assert(first <= size());
    assert(last <= size());
    assert(first <= last);
    size_type ret = 0;
    for (const auto &v : matrix_) {
      if (x & v.bit) {
        first = v.rank(first) + v.cnt;
        last = v.rank(last) + v.cnt;
      } else {
        ret += v.rank(last) - v.rank(first);
        first -= v.rank(first);
        last -= v.rank(last);
      }
    }
    return ret;
  }
  size_type at_most(size_type first, size_type last, const value_type x) const {
    assert(first <= size());
    assert(last <= size());
    assert(first <= last);
    size_type ret = 0;
    for (const auto &v : matrix_) {
      if (x & v.bit) {
        ret += last - first + v.rank(first) - v.rank(last);
        first = v.rank(first) + v.cnt;
        last = v.rank(last) + v.cnt;
      } else {
        first -= v.rank(first);
        last -= v.rank(last);
      }
    }
    return ret + (last - first);
  }
  size_type rangefreq(size_type first, size_type last, const value_type lower,
                      const value_type upper) const {
    assert(first <= size());
    assert(last <= size());
    assert(first <= last);
    assert(lower <= upper);
    return at_least(first, last, lower) - at_least(first, last, upper);
  }
  value_type successor(const size_type first, const size_type last,
                       const value_type x) const {
    assert(first <= size());
    assert(last <= size());
    assert(first <= last);
    const size_type k = at_least(first, last, x);
    return k ? quantile(first, last, k - 1) : not_found_;
  }
  value_type predecessor(const size_type first, const size_type last,
                         const value_type x) const {
    assert(first <= size());
    assert(last <= size());
    assert(first <= last);
    const size_type k = greater_than(first, last, x);
    return last - first == k ? not_found_ : quantile(first, last, k);
  }
  value_type strict_succ(const size_type first, const size_type last,
                         const value_type x) const {
    assert(first <= size());
    assert(last <= size());
    assert(first <= last);
    const size_type k = greater_than(first, last, x);
    return k ? quantile(first, last, k - 1) : not_found_;
  }
  value_type strict_pred(const size_type first, const size_type last,
                         const value_type x) const {
    assert(first <= size());
    assert(last <= size());
    assert(first <= last);
    const size_type k = at_least(first, last, x);
    return last - first == k ? not_found_ : quantile(first, last, k);
  }
};

/*

verify:http://judge.u-aizu.ac.jp/onlinejudge/review.jsp?rid=2948867#1
      :https://beta.atcoder.jp/contests/abc091/submissions/2701933

template<typename Integral,
         ::std::size_t bitsize = ::std::numeric_limits<Integral>::digits()>
class wavelet_matrix;

wavelet_matrix は静的な非負整数列に対する区間クエリを処理するデータ構造です
空間計算量 O(N)


テンプレートパラメータ
-typename Integral
 要素となる整数型

-::std::size_t bitsize
 扱うbit幅 ([0, 2^bitsize) の範囲を扱います)


メンバ型
-value_type
 要素の型 (Integral)

-size_type
 符号なし整数型 (::std::size_t)


-メンバ関数
template <class InpuIter>
-(constructor) (InputIter first, InpuIter last, value_type not_found = 0)
 [first, last) の要素から wavelet_matrix を構築します
 負数が含まれる場合、要素の大小関係を扱う関数が正常に機能しません
 not_found は幾つかのメンバ関数で値が存在しないことを示す返り値に使われます
 時間計算量 O(N*bitsize)

-size ()->size_type
 要素数を返します
 時間計算量 O(1)

-empty ()->bool
 size()==0 と同値です
 時間計算量 O(1)

-access (size_type index)->value_type
 index で指定した要素を返します
 時間計算量 O(bitsize)

-operator[] (size_type index)->value_type
 access() と同値です
 時間計算量 O(bitsize)

-rank (size_type first, size_type last, value_type x)->size_type
 [first, last) に存在する x の数を返します
 時間計算量 O(bitsize)

-quantile (size_type first, size_type last, size_type k)->value_type
 [first, last) で k 番目 (0-indexed) に大きい値を返します
 時間計算量 O(bitsize)

-rquantile (size_type first, size_type last, size_type k)->value_type
 [first, last) で k 番目 (0-indexed) に小さい値を返します
 時間計算量 O(bitsize)

-less_than (size_type first, size_type last, value_type x)->size_type
 [first, last) で x より小さい要素の数を返します
 時間計算量 O(bitsize)

-at_least (size_type first, size_type last, value_type x)->size_type
 [first, last) で x 以上の要素の数を返します
 時間計算量 O(bitsize)

-greater_than (size_type first, size_type last, value_type x)->size_type
 [first, last) で x より大きい要素の数を返します
 時間計算量 O(bitsize)

-at_most (size_type first, size_type last, value_type x)->size_type
 [first, last) で x 以下の要素の数を返します
 時間計算量 O(bitsize)

-rangefreq (size_type first, size_type last,
            value_type lower, value_type upper)->size_type
 [first, last) で lower 以上 upper 未満の要素の数を返します
 時間計算量 O(bitsize)

-successor (size_type first, size_type last, value_type x)->value_type
 [first, last) で x 以上で最小の要素を返します
 存在しない場合 not_found を返します
 時間計算量 O(bitsize)

-predecessor (size_type first, size_type last, value_type x)->value_type
 [first, last) で x 以下で最大の要素を返します
 存在しない場合 not_found を返します
 時間計算量 O(bitsize)

-strict_succ (size_type first, size_type last, value_type x)->value_type
 [first, last) で x より大きい最小の要素を返します
 存在しない場合 not_found を返します
 時間計算量 O(bitsize)

-strict_pred (size_type first, size_type last, value_type x)->value_type
 [first, last) で x より小さい最大の要素を返します
 存在しない場合 not_found を返します
 時間計算量 O(bitsize)


※N:全体の要素数
※popcount64() の時間計算量を O(1) と仮定

*/