#include <cassert>
#include <cstddef>
#include <functional>
#include <memory>
#include <utility>

template <class T, class Compare = ::std::less_equal<T>> class pairing_heap {

public:
  using value_type = T;
  using const_reference = const value_type &;
  using size_type = ::std::size_t;
  using value_compare = Compare;

private:
  struct node_type {
    value_type value;
    ::std::unique_ptr<node_type> left, right;
    template <class... Args>
    node_type(Args &&... args)
        : value(::std::forward<Args>(args)...), left(), right() {}
  };
  using pointer = ::std::unique_ptr<node_type>;
  pointer root;
  value_compare comp;
  size_type s;
  pointer merge(pointer x, pointer y) {
    if (!x)
      return ::std::move(y);
    if (!y)
      return ::std::move(x);
    if (!comp(x->value, y->value))
      x.swap(y);
    y->right = ::std::move(x->left);
    x->left = ::std::move(y);
    return ::std::move(x);
  }
  pointer mergelist(pointer curr) {
    pointer head, temp, next;
    while (curr) {
      next = ::std::move(curr->right);
      if (next) {
        temp = ::std::move(next->right);
        curr = merge(::std::move(curr), ::std::move(next));
      }
      curr->right = ::std::move(head);
      head = ::std::move(curr);
      curr = ::std::move(temp);
    }
    while (head) {
      next = ::std::move(head->right);
      curr = merge(::std::move(curr), ::std::move(head));
      head = ::std::move(next);
    }
    return ::std::move(curr);
  }

public:
  pairing_heap() : root(), comp(), s(0) {}
  explicit pairing_heap(const value_compare &x) : root(), comp(x), s(0) {}

  bool empty() const noexcept { return !root; }
  size_type size() const noexcept { return s; }

  const_reference top() const noexcept {
    assert(!empty());
    return root->value;
  }

  void push(const value_type &x) {
    root = merge(::std::move(root), ::std::make_unique<node_type>(x));
    ++s;
  }
  void push(value_type &&x) {
    root =
        merge(::std::move(root), ::std::make_unique<node_type>(::std::move(x)));
    ++s;
  }
  template <class... Args> void emplace(Args &&... args) {
    root = merge(::std::move(root),
                 ::std::make_unique<node_type>(::std::forward<Args>(args)...));
    ++s;
  }

  void pop() {
    assert(!empty());
    root = mergelist(::std::move(root->left));
    --s;
  }

  void meld(pairing_heap &x) {
    s += x.s;
    x.s = 0;
    root = merge(::std::move(root), ::std::move(x.root));
  }
  pairing_heap &operator+=(pairing_heap &x) {
    meld(x);
    return *this;
  }
};

/*

verify:https://beta.atcoder.jp/contests/apc001/submissions/2751553
      :https://beta.atcoder.jp/contests/arc098/submissions/2751569

template<class T, class Compare = ::std::less_equal<T>>
class pairing_heap;

pairing_heap は融合可能なヒープ(優先度付きキュー)です
空間計算量 O(N)


テンプレートパラメータ
-class T
 要素の型

-class Compare
 T が全順序集合を成すように大小比較を行うクラス
 デフォルトでは ::std::less_equal<T> で最小ヒープ(昇順)です


メンバ型
-value_type
 要素の型 (T)

-const_reference
 要素(value_type)へのconst参照型 (const value_type &)

-size_type
 要素数を表す符号なし整数型 (::std::size_t)

-value_compare
 比較クラスの型 (Compare)


メンバ関数
-(constructor) (const value_compare &x)
 x を比較関数として、空のヒープを構築します
 デフォルトでは value_compare() が使用されます
 時間計算量 O(1)

-empty ()->bool
 ヒープが空かどうかを返します
 時間計算量 O(1)

-size ()->size_type
 要素数を返します
 時間計算量 O(1)

-top ()->const_reference
 先頭の要素のconst参照を返します
 時間計算量 O(1)

-push (const value_type &x)
 x を要素として追加します
 時間計算量 O(1)

-template<class... Args>
 emplace (Args&&... args)
 コンストラクタの引数から直接構築で要素を追加します
 時間計算量 O(1)

-pop ()
 先頭の要素を削除します
 時間計算量 O(logN)

-meld (pairing_heap &x)
 x の全要素を追加します
 x は空になります
 時間計算量 O(1)

-operator+= (pairing_heap &x)->pairing_heap &
 x の全要素を追加します
 x は空になります
 本体の参照を返します
 時間計算量 O(1)


※N:要素数
※比較クラスの時間計算量を O(1) と仮定

*/
