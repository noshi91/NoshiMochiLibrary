#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

template <class Integral, ::std::size_t Bitlength, Integral None>
class wavelet_matrix {
public:
  using value_type = Integral;
  using size_type = ::std::size_t;
  static constexpr value_type none = None;

private:
  class bitvector {
    using bitfield = ::std::uint_least64_t;
    static constexpr ::std::size_t wardsize = 64;
    static ::std::size_t popcount(bitfield c) {
#ifdef __has_builtin
      return __builtin_popcountll(c);
#else
      c = (c & 0x5555555555555555ULL) + (c >> 1 & 0x5555555555555555ULL);
      c = (c & 0x3333333333333333ULL) + (c >> 2 & 0x3333333333333333ULL);
      c = (c + (c >> 4)) & 0x0F0F0F0F0F0F0F0FULL;
      return static_cast<::std::size_t>(c * 0x0101010101010101ULL >> 56 & 0x7f);
#endif
    }
    using value_type = typename wavelet_matrix::value_type;
    using size_type = typename wavelet_matrix::size_type;
    ::std::vector<::std::pair<bitfield, size_type>> dic;

  public:
    size_type cnt;
    value_type bit;
    constexpr bitvector() : dic(), cnt(0), bit(0) {}
    bitvector(const size_type size, const value_type b)
        : dic(size / wardsize + 1, {0, 0}), cnt(0), bit(b) {}
    void set(const size_type index) {
      dic[index / wardsize].first |= static_cast<bitfield>(1)
                                     << (index % wardsize);
    }
    void build() {
      const size_type len = dic.size();
      for (size_type i = 1; i < len; ++i)
        dic[i].second = dic[i - 1].second + popcount(dic[i - 1].first);
    }
    size_type rank(const size_type last) const {
      return dic[last / wardsize].second +
             popcount(dic[last / wardsize].first &
                      (static_cast<bitfield>(1) << (last % wardsize)) - 1);
    }
    bool access(const size_type index) const {
      return dic[index / wardsize].first >> (index % wardsize) & 1;
    }
  };
  static bool valid(const value_type value) {
    return value == none || !(value >> (Bitlength - 1) >> 1);
  }
  ::std::array<bitvector, Bitlength> matrix;
  size_type size_;

  size_type at_least(size_type first, size_type last,
                     const value_type value) const {
    assert(first <= size());
    assert(last <= size());
    assert(first <= last);
    size_type ret = 0;
    for (const auto &v : matrix) {
      const size_type l = v.rank(first), r = v.rank(last);
      if (value & v.bit) {
        first = l + v.cnt;
        last = r + v.cnt;
      } else {
        ret += r - l;
        first -= l;
        last -= r;
      }
    }
    return ret + last - first;
  }

public:
  constexpr wavelet_matrix() : matrix(), size_(0) {}
  template <class InputIter>
  wavelet_matrix(InputIter first, InputIter last) : matrix(), size_(0) {
    ::std::vector<value_type> vec0(first, last);
    const size_type len = vec0.size();
    size_ = len;
    ::std::vector<value_type> vec_l(len), vec_r(len);
    size_type l, r;
    value_type temp = static_cast<value_type>(1) << (Bitlength - 1);
    for (auto &v : matrix) {
      v = bitvector(len, temp);
      temp >>= 1;
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
      ::std::copy(vec_r.cbegin(), vec_r.cbegin() + r, vec0.begin() + l);
    }
  }

  size_type size() const noexcept { return size_; }
  bool empty() const noexcept { return size() == 0; }

  value_type operator[](size_type index) const {
    assert(index < size());
    value_type ret = 0;
    for (const auto &v : matrix)
      if (v.access(index))
        ret |= v.bit, index = v.rank(index) + v.cnt;
      else
        index -= v.rank(index);
    return ret;
  }
  value_type at(const size_type index) const {
    if (index < size())
      return operator[](index);
    else
      throw ::std::out_of_range("index out of range");
  }

  size_type rank(size_type first, size_type last,
                 const value_type value) const {
    assert(first <= size());
    assert(last <= size());
    assert(first <= last);
    assert(valid(value));
    for (const auto &v : matrix)
      if (value & v.bit)
        first = v.rank(first) + v.cnt, last = v.rank(last) + v.cnt;
      else
        first -= v.rank(first), last -= v.rank(last);
    return last - first;
  }
  value_type quantile(size_type first, size_type last, size_type k = 0,
                      const value_type upper = none) const {
    assert(first <= size());
    assert(last <= size());
    assert(valid(upper));
    if (upper != none)
      k += rangefreq(first, last, upper, none);
    if (last - first <= k)
      return none;
    value_type ret = 0;
    for (const auto &v : matrix) {
      const size_type l = v.rank(first), r = v.rank(last);
      if (r - l > k)
        first = l + v.cnt, last = r + v.cnt, ret |= v.bit;
      else
        first -= l, last -= r, k -= r - l;
    }
    return ret;
  }
  value_type rquantile(const size_type first, const size_type last,
                       size_type k = 0, const value_type lower = none) const {
    assert(first <= size());
    assert(last <= size());
    assert(first <= last);
    assert(valid(lower));
    if (lower != none)
      k += rangefreq(first, last, none, lower);
    if (last - first <= k)
      return none;
    return quantile(first, last, last - first - k - 1, none);
  }
  size_type rangefreq(const size_type first, const size_type last,
                      const value_type lower, const value_type upper) const {
    assert(first <= size());
    assert(last <= size());
    assert(first <= last);
    assert(valid(lower));
    assert(valid(upper));
    assert(lower == none || upper == none || lower <= upper);
    size_type ret = lower == none ? last - first : at_least(first, last, lower);
    return upper == none ? ret : ret - at_least(first, last, upper);
  }
};

/*

verify:http://judge.u-aizu.ac.jp/onlinejudge/review.jsp?rid=3121199#1
      :https://beta.atcoder.jp/contests/abc091/submissions/3139896

template<class Integral, ::std::size_t Bitlength, Integral None>
class wavelet_matrix;

wavelet_matrix は静的な非負整数列に対する区間クエリを処理するデータ構造です
空間計算量 O(N)


テンプレートパラメータ
-class Integral
 要素となる整数型

-::std::size_t Bitlength
 扱うbit幅 ([0, 2^Bitlength) の範囲を扱います)

-Integral None
 関数の引数や返り値において、値が存在しないことを表現することに使用します


メンバ型
-value_type
 要素の型 (Integral)

-size_type
 符号なし整数型 (::std::size_t)


メンバ定数
-none
 None


-メンバ関数
template <class InpuIter>
-(constructor) (InputIter first, InpuIter last)
 [first, last) の要素から wavelet_matrix を構築します
 負数が含まれる場合、要素の大小関係を扱う関数が正常に機能しません
 時間計算量 O(N*Bitlength)

-size ()->size_type
 要素数を返します
 時間計算量 O(1)

-empty ()->bool
 size()==0 と同値です
 時間計算量 O(1)

-operator[] (size_type index)->value_type
 index で指定した要素の値を返します
 時間計算量 O(Bitlength)

-at (size_type index)->value_type
 index で指定した要素を返します
 index が範囲外の時、::std::out_of_range 例外を送出します
 時間計算量 O(Bitlength)

-rank (size_type first, size_type last, value_type value)->size_type
 [first, last) に存在する value の数を返します
 時間計算量 O(Bitlength)

-quantile (size_type first, size_type last, size_type k = 0,
           value_type lower = none)->value_type
 [first, last) で lower 以上の値のうち k 番目 (0-indexed) に大きい値を返します
 lower = none のとき、対象は [first, last) の全ての値となります
 該当する値が存在しない時、none を返します
 時間計算量 O(Bitlength)

-rquantile (size_type first, size_type last, size_type k = 0,
           value_type upper = none)->value_type
 [first, last) で upper 未満の値のうち k 番目 (0-indexed) に小さい値を返します
 upper = none のとき、対象は [first, last) の全ての値となります
 該当する値が存在しない時、none を返します
 時間計算量 O(Bitlength)

-rangefreq (size_type first, size_type last,
            value_type lower, value_type upper)->size_type
 [first, last) で lower 以上 upper 未満の要素の数を返します
 lower = none のとき下限は存在しません
 upper = none のとき上限は存在しません
 下限、上限が両方存在するとき、lower <= upper が要求されます
 時間計算量 O(Bitlength)

※N:全体の要素数
※popcount() の時間計算量を O(1) と仮定
  デフォルトの実装は O(logloglogN) です

*/
