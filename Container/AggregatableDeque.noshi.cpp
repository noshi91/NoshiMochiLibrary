#include <cassert>
#include <memory>
#include <stdexcept>
#include <utility>

template <class Semigroup, template <class> class Allocator = ::std::allocator>
class aggregatable_deque {
public:
  using value_structure = Semigroup;
  using value_type = typename value_structure::value_type;
  using const_reference = const value_type &;

private:
  class add_s_tag_t {};
  static constexpr add_s_tag_t add_s_tag{};
  class add_l_tag_t {};
  static constexpr add_l_tag_t add_l_tag{};
  class add_r_tag_t {};
  static constexpr add_r_tag_t add_r_tag{};
  class node_type {
  public:
    using value_structure = typename aggregatable_deque::value_structure;
    using value_type = typename value_structure::value_type;
    value_type value;
    value_type sum;
    template <class... Args>
    node_type(add_s_tag_t, Args &&... args)
        : value(::std::forward<Args>(args)...), sum(value) {}
    template <class... Args>
    node_type(add_r_tag_t, const value_type &a, Args &&... args)
        : value(::std::forward<Args>(args)...),
          sum(value_structure::operation(a, value)) {}
    template <class... Args>
    node_type(add_l_tag_t, const value_type &a, Args &&... args)
        : value(::std::forward<Args>(args)...),
          sum(value_structure::operation(value, a)) {}

    void set_sum() { sum = value; }
  };

public:
  using allocator_type = Allocator<node_type>;

private:
  using alct = ::std::allocator_traits<allocator_type>;
  using pointer = typename alct::pointer;

public:
  using size_type = typename alct::size_type;

private:
  allocator_type alc;
  pointer ptr;
  size_type f_pos, b_pos, mask, cnt_l, cnt_r;

  size_type succ(const size_type p) const { return p + 1 & mask; }
  size_type pred(const size_type p) const { return p - 1 & mask; }
  void check_extend() {
    if (!(size() & mask)) {
      pointer next = alct::allocate(alc, (mask + 1) << 1);
      for (size_type i = 0; i <= mask; ++i) {
        alct::construct(alc, next + i, ::std::move(ptr[f_pos + i & mask]));
        alct::destroy(alc, ptr + (f_pos + i & mask));
      }
      alct::deallocate(alc, ptr, mask + 1);
      ptr = next;
      f_pos = 0;
      b_pos = mask;
      mask = mask << 1 | 1;
    }
  }
  void recalc() {
    if (cnt_r) {
      size_type i = b_pos - cnt_r + 1;
      ptr[i & mask].set_sum();
      for (; i != b_pos; ++i)
        ptr[i + 1 & mask].sum = value_structure::operation(
            ptr[i & mask].sum, ptr[i + 1 & mask].value);
    }
    if (cnt_l) {
      size_type i = f_pos + cnt_l - 1;
      ptr[i & mask].set_sum();
      for (; i != f_pos; --i)
        ptr[i - 1 & mask].sum = value_structure::operation(
            ptr[i - 1 & mask].value, ptr[i & mask].sum);
    }
  }

public:
  aggregatable_deque() : aggregatable_deque(allocator_type()) {}
  explicit aggregatable_deque(const allocator_type &a)
      : alc(a), ptr(alct::allocate(alc, 2)), f_pos(0), b_pos(1), mask(1),
        cnt_l(0), cnt_r(0) {}

  size_type size() const { return cnt_l + cnt_r; }
  bool empty() const { return size() == 0; }

  const_reference at(const size_type index) const {
    if (index < size())
      return ptr[f_pos + index & mask].value;
    else
      throw ::std::out_of_range("index out of range");
  }
  const_reference operator[](const size_type index) const {
    assert(index < size());
    return ptr[f_pos + index & mask].value;
  }
  const_reference front() const {
    assert(!empty());
    return ptr[f_pos].value;
  }
  const_reference back() const {
    assert(!empty());
    return ptr[b_pos].value;
  }
  value_type aggregate() const {
    assert(!empty());
    if (cnt_l) {
      if (cnt_r)
        return value_structure::operation(ptr[f_pos].sum, ptr[b_pos].sum);
      else
        return ptr[f_pos].sum;
    } else {
      return ptr[b_pos].sum;
    }
  }

  void push_back(const value_type &x) {
    if (cnt_r)
      alct::construct(alc, ptr + succ(b_pos), add_r_tag, ptr[b_pos].sum, x);
    else
      alct::construct(alc, ptr + succ(b_pos), add_s_tag, x);
    b_pos = succ(b_pos);
    ++cnt_r;
    check_extend();
  }
  void push_back(value_type &&x) {
    if (cnt_r)
      alct::construct(alc, ptr + succ(b_pos), add_r_tag, ptr[b_pos].sum,
                      ::std::move(x));
    else
      alct::construct(alc, ptr + succ(b_pos), add_s_tag, ::std::move(x));
    b_pos = succ(b_pos);
    ++cnt_r;
    check_extend();
  }
  template <class... Args> void emplace_back(Args &&... args) {
    if (cnt_r)
      alct::construct(alc, ptr + succ(b_pos), add_r_tag, ptr[b_pos].sum,
                      ::std::forward<Args>(args)...);
    else
      alct::construct(alc, ptr + succ(b_pos), add_s_tag,
                      ::std::forward<Args>(args)...);
    b_pos = succ(b_pos);
    ++cnt_r;
    check_extend();
  }
  void pop_back() {
    assert(!empty());
    alct::destroy(alc, ptr + b_pos);
    b_pos = pred(b_pos);
    if (!cnt_r) {
      cnt_r = cnt_l >> 1;
      cnt_l = (cnt_l - 1) >> 1;
      recalc();
    } else {
      --cnt_r;
    }
  }
  void push_front(const value_type &x) {
    if (cnt_l)
      alct::construct(alc, ptr + pred(f_pos), add_l_tag, ptr[f_pos].sum, x);
    else
      alct::construct(alc, ptr + pred(f_pos), add_s_tag, x);
    f_pos = pred(f_pos);
    ++cnt_l;
    check_extend();
  }
  void push_front(value_type &&x) {
    if (cnt_l)
      alct::construct(alc, ptr + pred(f_pos), add_l_tag, ptr[f_pos].sum,
                      ::std::move(x));
    else
      alct::construct(alc, ptr + pred(f_pos), add_s_tag, ::std::move(x));
    f_pos = pred(f_pos);
    ++cnt_l;
    check_extend();
  }
  template <class... Args> void emplace_front(Args &&... args) {
    if (cnt_l)
      alct::construct(alc, ptr + pred(f_pos), add_l_tag, ptr[f_pos].sum,
                      ::std::forward<Args>(args)...);
    else
      alct::construct(alc, ptr + pred(f_pos), add_s_tag,
                      ::std::forward<Args>(args)...);
    f_pos = pred(f_pos);
    ++cnt_l;
    check_extend();
  }
  void pop_front() {
    assert(!empty());
    alct::destroy(alc, ptr + f_pos);
    f_pos = succ(f_pos);
    if (!cnt_l) {
      cnt_l = cnt_r >> 1;
      cnt_r = cnt_r - 1 >> 1;
      recalc();
    } else {
      --cnt_l;
    }
  }
};

/*

verify:http://judge.u-aizu.ac.jp/onlinejudge/review.jsp?rid=3098277#1

template<class Semigroup, template <class> class Allocator = ::std::allocator>
class aggregatable_deque;

aggregatable_deque は半群列の総和の計算と deque の一部操作が可能なデータ構造です
空間計算量 O(N)


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

-template <class> class Allocator
 内部で使用するアロケータ


メンバ型
-value_structure
 構造の型 (Semigroup)

-value_type
 要素となる台集合の型 (value_structure::value_type)

-const_reference
 要素へのconst参照型 (const value_type &)

-allocator_type
 内部で使用するアロケータの型

-size_type
 符号なし整数型 (::std::allocator_traits<allocator_type>::size_type)


メンバ関数
-(constructor) ()
 空の状態で aggregatable_deque を構築します
 アロケータを渡すことも可能です
 時間計算量 O(1)

-empty ()->bool
 要素が空かどうか判定します
 時間計算量 O(1)

-size ()->size_type
 要素数を返します
 時間計算量 O(1)

-at (size_type index)->const_reference
 index で指定した要素にアクセスします
 範囲外の場合、::std::out_of_range 例外を送出します
 時間計算量 O(1)

-operator[] (size_type index)->const_reference
 index で指定した要素にアクセスします
 時間計算量 O(1)

-front ()->const_reference
 先頭の要素を返します
 時間計算量 O(1)

-back ()->const_reference
 末尾の要素を返します
 時間計算量 O(1)

-aggregate ()->value_type
 全要素の和を返します
 要素が空の場合の動作は定義されていません
 時間計算量 O(1)

-push_back (value_type x)
 末尾に要素を追加します
 時間計算量 償却 O(1)

template<class ...Args>
-emplace_back (Args&& ...args)
 末尾に要素を直接構築で追加します
 時間計算量 償却 O(1)

-pop_back ()
 末尾の要素を削除します
 時間計算量 償却 O(1)

-push_front (value_type x)
 先頭に要素を追加します
 時間計算量 償却 O(1)

template<class ...Args>
-emplace_front (Args&& ...args)
 先頭に要素を直接構築で追加します
 時間計算量 償却 O(1)

-pop_front ()
 先頭の要素を削除します
 時間計算量 償却 O(1)


※ N:全体の要素数
※ allocator_type::allocate()
   allocator_type::deallocate()
   以上の関数の時間/空間計算量を O(N) と仮定
※ その他各関数の時間計算量を O(1) と仮定

*/
