#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

template <class Compare> class rmq {
  using value_compare = Compare;
  static constexpr ::std::size_t BLOCKSIZE = 16;
  using block_type = ::std::uint_least16_t;
  static_assert(BLOCKSIZE <= ::std::numeric_limits<block_type>::digits, "");
  using size_type = ::std::size_t;

private:
  static size_type msb(size_type c) {
#ifdef __has_builtin
    return 31 - __builtin_clz(c);
#else
    ::std::size_t ret = 0;
    if (c >> 8)
      c >>= 8, ret += 8;
    if (c >> 4)
      c >>= 4, ret += 4;
    if (c >> 2)
      c >>= 2, ret += 2;
    return ret + (c >> 1);
#endif
  }
  static size_type ctz(const block_type c) {
#ifdef __has_builtin
    return __builtin_ctz(c);
#else
    static const ::std::array<size_type, 16> table = {
        0, 1, 2, 5, 3, 9, 6, 11, 15, 4, 8, 10, 14, 7, 13, 12};
    return table[(c & ~c + 1) * 0x9AFU >> 12 & 0xF];
#endif
  }

  value_compare comp;
  ::std::vector<block_type> small;
  ::std::vector<::std::vector<size_type>> large;

  size_type get_min(const size_type left, const size_type right) const {
    return comp(left, right) ? left : right;
  }

public:
  rmq() : comp(), small(), large() {}
  explicit rmq(const size_type size, const value_compare &x)
      : comp(x), small(), large() {
    ::std::vector<size_type> st;
    st.reserve(BLOCKSIZE);
    large.emplace_back();
    large.front().reserve(size / BLOCKSIZE);
    small.reserve(size);
    for (size_type i = 0; i < size; ++i) {
      while (!st.empty() && !comp(st.back(), i))
        st.pop_back();
      small.emplace_back(st.empty() ? 0 : small[st.back()]);
      small.back() |= static_cast<block_type>(1) << (i % BLOCKSIZE);
      st.emplace_back(i);
      if (!((i + 1) % BLOCKSIZE)) {
        large.front().emplace_back(st.front());
        st.clear();
      }
    }
    for (size_type i = 1; i << 1 <= size / BLOCKSIZE; i <<= 1) {
      ::std::vector<size_type> v;
      const size_type csz = size / BLOCKSIZE + 1 - (i << 1);
      v.reserve(csz);
      for (size_type k = 0; k < csz; ++k)
        v.emplace_back(get_min(large.back()[k], large.back()[k + i]));
      large.emplace_back(::std::move(v));
    }
  }

  size_type size() const { return small.size(); }

  size_type fold_closed(const size_type first, const size_type last) const {
    assert(first <= last);
    assert(last < size());

    const size_type left = first / BLOCKSIZE + 1;
    const size_type right = last / BLOCKSIZE;
    if (left < right) {
      const size_type p = msb(right - left);
      return get_min(
          get_min((left - 1) * BLOCKSIZE +
                      ctz(small[left * BLOCKSIZE - 1] &
                          ~static_cast<block_type>(0) << first % BLOCKSIZE),
                  large[p][left]),
          get_min(large[p][right - (static_cast<size_type>(1) << p)],
                  right * BLOCKSIZE + ctz(small[last])));
    } else if (left == right) {
      return get_min((left - 1) * BLOCKSIZE +
                         ctz(small[left * BLOCKSIZE - 1] &
                             ~static_cast<block_type>(0) << first % BLOCKSIZE),
                     left * BLOCKSIZE + ctz(small[last]));
    } else {
      return right * BLOCKSIZE + ctz(small[last] & ~static_cast<block_type>(0)
                                                       << first % BLOCKSIZE);
    }
  }
};

template <class Compare>
rmq<Compare> make_rmq(const ::std::size_t size, const Compare &x) {
  return rmq<Compare>(size, x);
}

/*

verify:http://judge.u-aizu.ac.jp/onlinejudge/review.jsp?rid=3094916#1

template<class Compare>
class rmq;

rmqは静的な列の最小値を高速に計算するデータ構造です
空間計算量 O(NlogN)


テンプレートパラメータ
-class Compare
 要素を比較する比較関数
 2つのインデックスを引数に取り、左の要素が小さいかどうかを真偽値で返します
 渡される引数は常に左側の方が等しいか小さいことが保証されます


メンバ型
-value_compare
 比較関数の型 (Compare)

-block_type
 内部実装で使用する符号なし整数型

-size_type
 インデックスに使用する符号なし整数型


メンバ関数
-(constructor) (size_type size, value_compare x)
 要素列長と比較関数を受け取り構築します
 時間計算量 O(N)

-size ()->size_type
 要素数を返します
 時間計算量 O(1)

-fold_closed (size_type first, size_type last)->size_type
 [first, last] で最小値を与えるインデックスを返します。
 時間計算量 O(1) ※1


非メンバ関数
template<class Compare>
-make_rmq (::std::size_t size, Compare x)->rmq<Compare>
 rmq を構築するヘルパ関数


※N:全体の要素数
※1 msb() の時間計算量を O(1) と仮定 - bit演算の場合 O(loglogN)

*/
