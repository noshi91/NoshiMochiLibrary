#include <cassert>
#include <memory>
#include <type_traits>
#include <utility>

template <class T> class persistent_queue {
public:
  static_assert(::std::is_copy_constructible<T>::value,
                "T must be copy constructible");

  using value_type = T;
  using const_reference = const value_type &;

private:
  class node_type;
  using node_ptr = ::std::shared_ptr<node_type>;
  using raw_ptr = node_type *;
  class stream_type;
  using stream_ptr = ::std::shared_ptr<stream_type>;

  template <class... Args> static node_ptr make_node(Args &&... args) {
    return ::std::make_shared<node_type>(::std::forward<Args>(args)...);
  }
  template <class... Args> static stream_ptr make_stream(Args &&... args) {
    return ::std::make_shared<stream_type>(::std::forward<Args>(args)...);
  }

  class node_type {
  public:
    template <class Nptr, class... Args>
    node_type(Nptr &&n, Args &&... args)
        : next(::std::forward<Nptr>(n)), value(::std::forward<Args>(args)...) {}
    node_ptr next;
    value_type value;
  };
  class stream_type {
  public:
    template <class Rptr>
    stream_type(const node_ptr &s, Rptr &&r)
        : scan(s), rotate(::std::forward<Rptr>(r)) {}

    node_ptr scan, rotate;

    node_ptr next() {
      node_ptr ret;
      if (scan) {
        ret = make_node(nullptr, scan->value);
        scan = scan->next;
      } else {
        while (rotate) {
          ret = make_node(::std::move(ret), rotate->value);
          rotate = rotate->next;
        }
      }
      return ::std::move(ret);
    }
  };
  node_ptr f_st;
  raw_ptr proc;
  stream_ptr stream;
  node_ptr b_st;

  explicit persistent_queue(::std::shared_ptr<stream_type> &&st)
      : f_st(st->next()), proc(f_st.get()), stream(::std::move(st)), b_st() {}
  template <class Bptr>
  explicit persistent_queue(const node_ptr &f, const raw_ptr &p,
                            const stream_ptr &s, Bptr &&b)
      : f_st(f), proc(p), stream(s), b_st(::std::forward<Bptr>(b)) {}

  template <class... Args>
  persistent_queue push_internal(Args &&... args) const {
    if (!proc) {
      return persistent_queue(
          make_stream(f_st, make_node(b_st, ::std::forward<Args>(args)...)));
    }
    if (!proc->next) {
      proc->next = stream->next();
    }
    return persistent_queue(f_st, proc->next.get(), stream,
                            make_node(b_st, ::std::forward<Args>(args)...));
  }

public:
  constexpr persistent_queue() noexcept
      : f_st(), proc(nullptr), stream(), b_st() {}

  bool empty() const noexcept { return !static_cast<bool>(f_st); }

  const_reference front() const {
    assert(("called persistent_queue::front on an empty queue", !empty()));
    return f_st->value;
  }

  persistent_queue push(const value_type &x) const { return push_internal(x); }
  persistent_queue push(value_type &&x) const {
    return push_internal(::std::move(x));
  }
  template <class... Args> persistent_queue emplace(Args &&... args) const {
    return push_internal(::std::forward<Args>(args)...);
  }
  persistent_queue pop() const {
    assert(("called persistent_queue::pop on an empty queue", !empty()));
    if (!proc) {
      return persistent_queue(make_stream(f_st->next, b_st));
    }
    if (!proc->next) {
      proc->next = stream->next();
    }
    return persistent_queue(f_st->next, proc->next.get(), stream, b_st);
  }
};

/*

verify:http://judge.u-aizu.ac.jp/onlinejudge/review.jsp?rid=3377452#1


template<class T>
class persistent_queue;

persistent_queue は要素の first-in first-out を実現する永続的データ構造です
空間計算量 操作列に依存


テンプレートパラメータ
-class T
 要素となる型
 コピー構築可能である必要があります


メンバ型
-value_type
 要素の型 (T)

-const_reference
 要素への const 参照型 (const value_type &)


メンバ関数
-(destructor) ()
 時間計算量 償却 Θ(1)

-empty () const -> bool
 queue が空かどうかを判定します
 時間計算量 Θ(1)

-front () const -> const_reference
 先頭の要素にアクセスします
 時間計算量 Θ(1)

-push (const value_type &x) const -> persistent_queue
-push (value_type &&x) const -> persistent_queue
 x を末尾に追加した queue を作成します
 時間/空間計算量 償却 Θ(1)

template <class... Args>
-emplace (Args &&... args) const -> persistent_queue
 args から直接構築で要素を末尾に追加した queue を作成します
 時間/空間計算量 償却 Θ(1)

-pop () const -> persistent_queue
 先頭要素を削除した queue を作成します
 時間/空間計算量 償却 Θ(1)

*/
