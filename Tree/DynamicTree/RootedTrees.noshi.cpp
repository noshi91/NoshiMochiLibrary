#include <cassert>
#include <utility>
#include <vector>

class rooted_trees {
private:
  class node_type;
  using container_type = ::std::vector<node_type>;

public:
  using size_type = typename container_type::size_type;

private:
  class node_type {
    using pointer = node_type *;
    pointer left, right, parent;
    bool reversed;

  public:
    node_type()
        : left(nullptr), right(nullptr), parent(nullptr), reversed(false) {}

    void reverse() { reversed = !reversed; }
    void cut_left() {
      if (left) {
        left->parent = nullptr;
        left = nullptr;
      }
    }
    void push() {
      if (reversed) {
        reversed = false;
        ::std::swap(left, right);
        if (left)
          left->reverse();
        if (right)
          right->reverse();
      }
    }
    void set_left_of(const pointer ptr) {
      ptr->left = this;
      parent = ptr;
    }
    void set_right_of(const pointer ptr) {
      ptr->right = this;
      parent = ptr;
    }
    void rotate_as_left(const pointer ptr) {
      if (right)
        right->set_left_of(ptr);
      else
        ptr->left = nullptr;
      ptr->set_right_of(this);
    }
    void rotate_as_right(const pointer ptr) {
      if (left)
        left->set_right_of(ptr);
      else
        ptr->right = nullptr;
      ptr->set_left_of(this);
    }
    void splay() {
      for (pointer x, y = this;;) {
        if (x = parent) {
          if (x->left == y) {
            if (y = x->parent) {
              if (y->left == x) {
                parent = y->parent;
                x->rotate_as_left(y);
                rotate_as_left(x);
              } else if (y->right == x) {
                parent = y->parent;
                rotate_as_left(x);
                rotate_as_right(y);
              } else {
                parent = y;
                rotate_as_left(x);
                return;
              }
            } else {
              parent = nullptr;
              rotate_as_left(x);
              return;
            }
          } else if (x->right == y) {
            if (y = x->parent) {
              if (y->left == x) {
                parent = y->parent;
                rotate_as_right(x);
                rotate_as_left(y);
              } else if (y->right == x) {
                parent = y->parent;
                x->rotate_as_right(y);
                rotate_as_right(x);
              } else {
                parent = y;
                rotate_as_right(x);
                return;
              }
            } else {
              parent = nullptr;
              rotate_as_right(x);
              return;
            }
          } else {
            return;
          }
        } else {
          return;
        }
      }
    }
    static void propagate(pointer ptr) {
      pointer prev = nullptr;
      while (ptr) {
        ::std::swap(ptr->parent, prev);
        ::std::swap(ptr, prev);
      }
      while (prev) {
        prev->push();
        ::std::swap(prev->parent, ptr);
        ::std::swap(prev, ptr);
      }
    }
    pointer expose() {
      propagate(this);
      pointer x = this, prev = nullptr;
      while (x) {
        x->splay();
        x->right = prev;
        prev = x;
        x = x->parent;
      }
      splay();
      return prev;
    }
    bool exposed_just_before() const { return !static_cast<bool>(parent); }
    bool is_root() const { return !static_cast<bool>(left); }
    pointer splay_prev() {
      node_type temp;
      pointer subtree = &temp;
      pointer cur = left, cur_r;
      left = nullptr;
      while (cur->push(), cur_r = cur->right) {
        cur_r->push();
        if (cur_r->right) {
          cur_r->rotate_as_right(cur);
          cur_r->set_right_of(subtree);
          subtree = cur_r;
          cur = cur_r->right;
        } else {
          cur->set_right_of(subtree);
          subtree = cur;
          cur = cur_r;
          break;
        }
      }
      cur->parent = nullptr;
      if (cur->left)
        cur->left->set_right_of(subtree);
      if (temp.right)
        temp.right->set_left_of(cur);
      else
        cur->left = nullptr;
      set_right_of(cur);
      return cur;
    }
  };
  using pointer = node_type *;

  container_type nodes;

  pointer get_ptr(const size_type v) { return &nodes[v]; }
  size_type get_index(const pointer p) {
    return static_cast<size_type>(p - nodes.data());
  }

public:
  explicit rooted_trees(const size_type size) : nodes(size) {}

  bool empty() const { return nodes.empty(); }
  size_type size() const { return nodes.size(); }

  bool is_root(const size_type v) {
    assert(v < size());
    nodes[v].expose();
    return nodes[v].is_root();
  }
  bool is_connected(const size_type v, const size_type w) {
    assert(v < size());
    assert(w < size());
    if (v == w)
      return true;
    nodes[v].expose();
    nodes[w].expose();
    return !nodes[v].exposed_just_before();
  }
  size_type lca(const size_type v, const size_type w) {
    assert(v < size());
    assert(w < size());
    if (v == w)
      return v;
    nodes[v].expose();
    pointer ptr = nodes[w].expose();
    if (nodes[v].exposed_just_before())
      return size();
    if (!static_cast<bool>(ptr))
      return w;
    return get_index(ptr);
  }
  size_type parent(const size_type v) {
    assert(v < size());
    nodes[v].expose();
    if (nodes[v].is_root())
      return size();
    else
      return get_index(nodes[v].splay_prev());
  }
  void reroot(const size_type v) {
    assert(v < size());
    nodes[v].expose();
    nodes[v].reverse();
  }
  void set_parent(const size_type v, const size_type p) {
    assert(v < size());
    assert(p < size());
    cut(v);
    assert(!is_connected(v, p));
    nodes[p].expose();
    nodes[p].set_left_of(get_ptr(v));
  }
  void cut(const size_type v) {
    assert(v < size());
    nodes[v].expose();
    nodes[v].cut_left();
  }

  ::std::vector<::std::pair<size_type, size_type>> all_edges() {
    ::std::vector<::std::pair<size_type, size_type>> ret;
    for (size_type i = static_cast<size_type>(0); i < size(); ++i) {
      const size_type p = parent(i);
      if (p != size())
        ret.emplace_back(p, i);
    }
    return ::std::move(ret);
  }
};

/*

verify:https://beta.atcoder.jp/contests/joisc2013-day4/submissions/3529891
       http://judge.u-aizu.ac.jp/onlinejudge/review.jsp?rid=3222968#1

class rooted_trees;

rooted_trees は動的な根付き木の集合の構造を管理するデータ構造です
空間計算量 O(N)


メンバ型
-size_type
 符号なし整数型 (container_type::size_type)


メンバ関数
-(constructor) (size_type size)
 頂点数 size で rooted_trees を構築します
 各頂点は独立した状態で初期化されます
 時間計算量 O(N)

-empty ()->bool
 集合が空かどうか判定します
 時間計算量 O(1)

-size ()->size_type
 頂点数を返します
 時間計算量 O(1)

-is_root (size_type v)->bool
 v が v の属する木において根かどうか判定します
 時間計算量 償却 O(logN)

-is_connected (size_type v, size_type w)->bool
 v と w が同じ木に属しているかどうか判定します
 時間計算量 償却 O(logN)

-lca (size_type v, size_type w)->size_type
 v と w の最低共通祖先を、
 存在しない場合 size() を返します
 時間計算量 償却 O(logN)

-parent (size_type v)->size_type
 v の親を、存在しない場合 size() を返します
 時間計算量 償却 O(logN)

-reroot (size_type v)
 v を v の属する木の根に変更します
 時間計算量 償却 O(logN)

-set_parent (size_type v, size_type p)
 v の親を p に変更します
 操作後、全体は森となっている必要があります
 時間計算量 償却 O(logN)

-cut (size_type v)
 v と v の親を繋ぐ辺を削除します
 時間計算量 償却 O(logN)

-all_edges 略


※N:全体の要素数

*/