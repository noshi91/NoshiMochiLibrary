#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

template <typename Integral,
          std::size_t bitsize = std::numeric_limits<Integral>::digits()>
class WaveletMatrix {
public:
  using value_type = Integral;
  using size_type = std::size_t;

private:
  using uint32 = std::uint_least32_t;
  static size_type popcount32(uint32 c) {
#ifdef __GNUC__
    return __builtin_popcount(c);
#elif _MSC_VER_
    return _mm_popcnt_u32(c);
#else
    c = (c & 0x55555555) + ((c >> 1) & 0x55555555);
    c = (c & 0x33333333) + ((c >> 2) & 0x33333333);
    c = (c + (c >> 4)) & 0x0F0F0F0F;
    c += c >> 8;
    c += c >> 16;
    return static_cast<size_type>(c) & 0x3F;
#endif
  }
  struct FID {
    std::vector<uint32> dic;
    size_type cnt;
    value_type bit;
    FID() {}
    FID(const size_type bsize) : dic(bsize, 0) {}
    void set(const size_type index) {
      dic[index >> 4 | 1] |= static_cast<uint32>(1) << (index & 0x1f);
    }
    void build() {
      const size_type len = dic.size();
      for (size_type j = 2; j < len; j += 2)
        dic[j] = dic[j - 2] + popcount32(dic[j - 1]);
    }
    size_type rank(const size_type last) const {
      return static_cast<size_type>(
                 dic[last >> 4 & ~static_cast<size_type>(1)]) +
             popcount32(dic[last >> 4 | 1] &
                        (static_cast<size_type>(1) << (last & 0x1f)) - 1);
    }
    bool access(const size_type index) const {
      return dic[index >> 4 | 1] >> (index & 0x1f) & 1;
    }
  };
  std::array<FID, bitsize> matrix;
  value_type NOT_FOUND;
  size_type size_;

public:
  WaveletMatrix(std::vector<value_type> data, const value_type NOT_FOUND = 0)
      : size_(data.size()), NOT_FOUND(NOT_FOUND) {
    const size_type len = data.size(), block = ((len >> 5) + 1) << 1;
    std::vector<value_type> L(len), R(len);
    size_type l, r;
    value_type temp = static_cast<value_type>(1) << (bitsize - 1);
    for (auto &x : matrix) {
      x = FID(block);
      x.bit = temp;
      temp = temp >> 1 & ~temp;
      l = 0;
      r = 0;
      for (size_type j = 0; j < len; ++j)
        if (data[j] & x.bit)
          R[r++] = data[j], x.set(j);
        else
          L[l++] = data[j];
      x.cnt = l;
      x.build();
      std::swap(data, L);
      std::copy(R.begin(), R.begin() + r, data.begin() + l);
    }
  }

  size_type size() const noexcept { return size_; }
  bool empty() const noexcept { return size() == 0; }

  value_type access(size_type index) const {
    assert(index < size());
    value_type ret = 0;
    for (const auto &x : matrix)
      if (x.access(index))
        ret |= x.bit, index = x.rank(index) + x.cnt;
      else
        index -= x.rank(index);
    return ret;
  }
  value_type operator[](const size_type index) const {
    assert(index < size());
    return access(index);
  }
  size_type rank(size_type first, size_type last, const value_type data) const {
    assert(first <= size());
    assert(last <= size());
    assert(first <= last);
    for (const auto &x : matrix)
      if (data & x.bit)
        first = x.rank(first) + x.cnt, last = x.rank(last) + x.cnt;
      else
        first -= x.rank(first), last -= x.rank(last);
    return last - first;
  }
  value_type quantile(size_type first, size_type last, size_type k) const {
    assert(first < size());
    assert(last <= size());
    assert(first < last);
    assert(last - first > k);
    value_type ret = 0;
    for (const auto &x : matrix) {
      const size_type l = x.rank(first), r = x.rank(last);
      if (r - l > k)
        first = l + x.cnt, last = r + x.cnt, ret |= x.bit;
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
                      const value_type data) const {
    assert(first <= size());
    assert(last <= size());
    assert(first <= last);
    size_type ret = 0;
    for (const auto &x : matrix) {
      if (data & x.bit) {
        ret += last - first + x.rank(first) - x.rank(last);
        first = x.rank(first) + x.cnt;
        last = x.rank(last) + x.cnt;
      } else {
        first -= x.rank(first);
        last -= x.rank(last);
      }
    }
    return ret;
  }
  size_type at_least(size_type first, size_type last,
                     const value_type data) const {
    assert(first <= size());
    assert(last <= size());
    assert(first <= last);
    size_type ret = 0;
    for (const auto &x : matrix) {
      if (data & x.bit) {
        first = x.rank(first) + x.cnt;
        last = x.rank(last) + x.cnt;
      } else {
        ret += x.rank(last) - x.rank(first);
        first -= x.rank(first);
        last -= x.rank(last);
      }
    }
    return ret + (last - first);
  }
  size_type greater_than(size_type first, size_type last,
                         const value_type data) const {
    assert(first <= size());
    assert(last <= size());
    assert(first <= last);
    size_type ret = 0;
    for (const auto &x : matrix) {
      if (data & x.bit) {
        first = x.rank(first) + x.cnt;
        last = x.rank(last) + x.cnt;
      } else {
        ret += x.rank(last) - x.rank(first);
        first -= x.rank(first);
        last -= x.rank(last);
      }
    }
    return ret;
  }
  size_type at_most(size_type first, size_type last,
                    const value_type data) const {
    assert(first <= size());
    assert(last <= size());
    assert(first <= last);
    size_type ret = 0;
    for (const auto &x : matrix) {
      if (data & x.bit) {
        ret += last - first + x.rank(first) - x.rank(last);
        first = x.rank(first) + x.cnt;
        last = x.rank(last) + x.cnt;
      } else {
        first -= x.rank(first);
        last -= x.rank(last);
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
                       const value_type data) const {
    assert(first <= size());
    assert(last <= size());
    assert(first <= last);
    const size_type k = at_least(first, last, data);
    return k ? quantile(first, last, k - 1) : NOT_FOUND;
  }
  value_type predecessor(const size_type first, const size_type last,
                         const value_type data) const {
    assert(first <= size());
    assert(last <= size());
    assert(first <= last);
    const size_type k = greater_than(first, last, data);
    return last - first == k ? NOT_FOUND : quantile(first, last, k);
  }
  value_type strict_succ(const size_type first, const size_type last,
                         const value_type data) const {
    assert(first <= size());
    assert(last <= size());
    assert(first <= last);
    const size_type k = greater_than(first, last, data);
    return k ? quantile(first, last, k - 1) : NOT_FOUND;
  }
  value_type strict_pred(const size_type first, const size_type last,
                         const value_type data) const {
    assert(first <= size());
    assert(last <= size());
    assert(first <= last);
    const size_type k = at_least(first, last, data);
    return last - first == k ? NOT_FOUND : quantile(first, last, k);
  }
};

/*

verify:http://judge.u-aizu.ac.jp/onlinejudge/review.jsp?rid=2835080#1

template<typename Integral,
         std::size_t bitsize = std::numeric_limits<Integral>::digits()>
class WaveletMatrix;

WaveletMatrix は静的な非負整数列に対する区間クエリを処理するデータ構造です
空間計算量 O(N)


テンプレートパラメータ
-typename Integral
 要素となる整数型

-std::size_t bitsize
 扱うbit幅 ([0, 2^bitsize) の範囲を扱います)


メンバ型
-value_type
 要素の型 (Integral)

-size_type
 符号なし整数型 (std::size_t)


-メンバ関数
-(constructor) (std::vector<value_type> data, value_type NOT_FOUND = 0)
 data の各要素を元に WaveletMatrix を構築します
 負数が含まれる場合、要素の大小関係を扱う関数が正常に機能しません
 NOT_FOUND は幾つかのメンバ関数で値が存在しないことを示す返り値に使われます
 時間計算量 O(N*bitsize)

-size ()->size_type
 要素数を返します
 時間計算量 O(1)

-empty ()->bool
 size()==0 と同値です
 時間計算量 O(1)

-access (size_type index)->value_type
 index で指定した要素を返します
 時間計算量 O(loglogN*bitsize)

-operator[] (size_type index)->value_type
 access() と同値です
 時間計算量 O(loglogN*bitsize)

-rank (size_type first, size_type last, value_type data)->size_type
 [first, last) に存在する data の数を返します
 時間計算量 O(loglogN*bitsize)

-quantile (size_type first, size_type last, size_type k)->value_type
 [first, last) で k 番目 (0-indexed) に大きい値を返します
 時間計算量 O(loglogN*bitsize)

-rquantile (size_type first, size_type last, size_type k)->value_type
 [first, last) で k 番目 (0-indexed) に小さい値を返します
 時間計算量 O(loglogN*bitsize)

-less_than (size_type first, size_type last, value_type data)->size_type
 [first, last) で data より小さい要素の数を返します
 時間計算量 O(loglogN*bitsize)

-at_least (size_type first, size_type last, value_type data)->size_type
 [first, last) で data 以上の要素の数を返します
 時間計算量 O(loglogN*bitsize)

-greater_than (size_type first, size_type last, value_type data)->size_type
 [first, last) で data より大きい要素の数を返します
 時間計算量 O(loglogN*bitsize)

-at_most (size_type first, size_type last, value_type data)->size_type
 [first, last) で data 以下の要素の数を返します
 時間計算量 O(loglogN*bitsize)

-rangefreq (size_type first, size_type last,
            value_type lower, value_type upper)->size_type
 [first, last) で lower 以上 upper 未満の要素の数を返します
 時間計算量 O(loglogN*bitsize)

-successor (size_type first, size_type last, value_type data)->value_type
 [first, last) で data 以上で最小の要素を返します
 存在しない場合 NOT_FOUND を返します
 時間計算量 O(loglogN*bitsize)

-predecessor (size_type first, size_type last, value_type data)->value_type
 [first, last) で data 以下で最大の要素を返します
 存在しない場合 NOT_FOUND を返します
 時間計算量 O(loglogN*bitsize)

-strict_succ (size_type first, size_type last, value_type data)->value_type
 [first, last) で data より大きい最小の要素を返します
 存在しない場合 NOT_FOUND を返します
 時間計算量 O(loglogN*bitsize)

-strict_pred (size_type first, size_type last, value_type data)->value_type
 [first, last) で data より小さい最大の要素を返します
 存在しない場合 NOT_FOUND を返します
 時間計算量 O(loglogN*bitsize)


※N:全体の要素数

*/