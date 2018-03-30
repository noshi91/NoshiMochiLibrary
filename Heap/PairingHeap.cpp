#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <utility>

template <typename T, class Compare = std::greater<T>> class PairingHeap {
  static constexpr std::uint_fast64_t ALLOCSIZE = (std::uint_fast64_t)1 << 10;

public:
  using value_type = T;
  using reference = value_type &;
  using const_reference = const value_type &;
  using size_type = std::uint_fast32_t;

private:
  struct node_t {
    value_type data;
    node_t *left, *right;
  };
  node_t *root;
  size_type size_;
  static node_t *merge(node_t *x, node_t *y) {
    static const Compare comp = Compare();
    if (comp(y->data, x->data))
      std::swap(x, y);
    x->right = y->left;
    y->left = x;
    return y;
  }

public:
  PairingHeap() : root(nullptr), size_(0) {}
  const_reference top() const {
    assert(root);
    return root->data;
  }
  const_reference pop() {
    assert(root);
    --size_;
    const_reference ret = root->data;
    node_t *n = nullptr, *x = root->left, *y;
    while (x) {
      y = x;
      if (x->right) {
        x = x->right->right;
        y = merge(y, y->right);
      } else
        x = nullptr;
      y->right = n;
      n = y;
    }
    if (!n) {
      root = nullptr;
      return ret;
    }
    root = n;
    n = n->right;
    while (n) {
      x = n;
      n = n->right;
      root = merge(root, x);
    }
    return ret;
  }
  void push(const_reference data) {
    static node_t *pool = nullptr;
    static std::uint_fast64_t it = ALLOCSIZE;
    if (it == ALLOCSIZE) {
      pool = (node_t *)malloc(ALLOCSIZE * sizeof(node_t));
      it = 0;
    }
    node_t *top = &pool[it++];
    top->data = data;
    top->left = nullptr;
    root = root ? merge(root, top) : top;
    ++size_;
  }
  bool empty() const { return !root; }
  size_type size() const { return size_; }
  void meld(PairingHeap<T, Compare> &other) {
    size_ += other.size_;
    other.size_ = 0;
    if (other.root) {
      root = root ? merge(root, other.root) : other.root;
      other.root = nullptr;
    }
  }
};

/*

verify:https://beta.atcoder.jp/contests/apc001/submissions/2280810

template<typename T, class Compare = std::greater<T>>
class PairingHeap;

PairingHeapは融合可能なヒープ(優先度付きキュー)です
空間計算量 O(N)


テンプレートパラメータ
-typename T
 operator< によって大小が定義された構造体
 要素の型になります

-class Compare
 大小比較を行うクラス
 デフォルトでは std::greater<T> で最小ヒープ(昇順)です


メンバ型
-value_type
 要素の型 (T)

-reference
 要素(value_type)への参照型 (value_type &)

-const_reference
 要素(value_type)へのconst参照型 (const value_type &)

-size_type
 要素数を表す符号なし整数型 (std::uint_fast32_t)

メンバ関数
-(constructor) ()
 空のヒープを構築します
 時間計算量 O(1)

-top (void)->const_reference
 先頭の要素のconst参照を返します
 時間計算量 O(1)

-pop (void)->const_reference
 先頭の要素を削除し、そのconst参照を返します
 時間計算量 償却 O(logN)

-push (const_reference data)
 data を要素としてヒープに追加します
 時間計算量 O(1)

-empty ()->bool
 ヒープが空かどうかを返します
 時間計算量 O(1)

-size ()->size_type
 要素数を返します
 時間計算量 O(1)

-meld (ParingHeap<T, Compare> &other)
 other の持つ要素全てをヒープに追加します
 other は空になります
 時間計算量 O(1)


※N:要素数
※Compare の時間計算量を O(1) と仮定

*/