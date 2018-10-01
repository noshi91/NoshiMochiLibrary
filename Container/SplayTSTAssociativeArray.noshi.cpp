#include <memory>
#include <type_traits>
#include <utility>

template <class Key, class T> class splay_tst_associative_array {
public:
  using key_type = Key;
  using value_type = T;
  using reference = value_type &;

  static_assert(::std::is_default_constructible<value_type>::value,
                "T must be default constructible");

private:
  using key_value = typename key_type::value_type;
  class node_type;
  class next_ptr {
  public:
    ::std::unique_ptr<node_type> ptr;
    typename splay_tst_associative_array::value_type value;
    next_ptr() : ptr(), value() {}
  };
  class node_type {
    using key_value = typename splay_tst_associative_array::key_value;

  public:
    key_value key;
    ::std::unique_ptr<node_type> left, right;
    next_ptr middle;
    node_type(const key_value &key_) : key(key_), left(), right(), middle() {}
  };
  using pointer = ::std::unique_ptr<node_type>;
  using p_ptr = pointer *;
  next_ptr root;

public:
  splay_tst_associative_array() : root() {}
  template <class InputIterator>
  splay_tst_associative_array(InputIterator first, InputIterator last)
      : root() {
    for (; first != last; ++first)
      operator[](first->first) = first->second;
  }

  bool empty() const { return !root.ptr; }

  reference operator[](const key_type &key) {
    next_ptr *prev = &root;
    for (const auto &elm : key) {
      pointer ptr = ::std::move(prev->ptr);
      pointer sub_l, sub_r;
      p_ptr left = &sub_l, right = &sub_r;
      while (1) {
        if (!ptr)
          ptr = ::std::make_unique<node_type>(elm);
        if (!(ptr->key <= elm)) {
          if (!ptr->left)
            ptr->left = ::std::make_unique<node_type>(elm);
          if (!(ptr->left->key <= elm)) {
            *right = ::std::move(ptr->left);
            ptr->left = ::std::move((*right)->right);
            (*right)->right = ::std::move(ptr);
            right = &(*right)->left;
            ptr = ::std::move(*right);
          } else if (!(elm <= ptr->left->key)) {
            *right = ::std::move(ptr);
            right = &(*right)->left;
            *left = ::std::move(*right);
            left = &(*left)->right;
            ptr = ::std::move(*left);
          } else {
            *right = ::std::move(ptr);
            right = &(*right)->left;
            ptr = ::std::move(*right);
          }
        } else if (!(elm <= ptr->key)) {
          if (!ptr->right)
            ptr->right = ::std::make_unique<node_type>(elm);
          if (!(ptr->right->key <= elm)) {
            *left = ::std::move(ptr);
            left = &(*left)->right;
            *right = ::std::move(*left);
            right = &(*right)->left;
            ptr = ::std::move(*right);
          } else if (!(elm <= ptr->right->key)) {
            *left = ::std::move(ptr->right);
            ptr->right = ::std::move((*left)->left);
            (*left)->left = ::std::move(ptr);
            left = &(*left)->right;
            ptr = ::std::move(*left);
          } else {
            *left = ::std::move(ptr);
            left = &(*left)->right;
            ptr = ::std::move(*left);
          }
        } else {
          *left = ::std::move(ptr->left);
          ptr->left = ::std::move(sub_l);
          *right = ::std::move(ptr->right);
          ptr->right = ::std::move(sub_r);
          break;
        }
      }
      prev->ptr = ::std::move(ptr);
      prev = &prev->ptr->middle;
    }
    return prev->value;
  }
};

/*

verify:http://judge.u-aizu.ac.jp/onlinejudge/review.jsp?rid=3164174#1
      :https://beta.atcoder.jp/contests/agc026/submissions/3315522

template<class Key, class T>
class splay_tst_associative_array;

splay_tst_associative_array は要素列をキーとする連想配列です
空間計算量 O(Σ|S|)


テンプレートパラメータ
-class Key
 キーとなる型
 range_based_for での走査が可能である必要があります

 以下のメンバを要求します
 -value_type
  要素の型
  operator<= により大小比較が可能である必要があります

-class T
 要素となる型
 デフォルト構築が可能である必要があります


メンバ型
-kry_type
 キーの型 (Key)

-value_type
 要素の型 (T)

-reference
 要素への参照型 (value_type &)


メンバ関数
-(constructor) ()
 空の状態で splay_tst_associative_array を構築します
 時間計算量 O(1)

-empty ()->bool
 要素が空かどうか判定します
 時間計算量 O(1)

-operator[] (key_type key)->reference
 key で指定した要素にアクセスします
 最初にアクセスしたときに要素はデフォルト構築されます
 時間計算量 償却 O(|key|+logN)


※ N:全体の要素数
※ |S|:要素列 S の長さ
※ キーの各要素の大小比較の時間計算量を O(1) と仮定

*/
