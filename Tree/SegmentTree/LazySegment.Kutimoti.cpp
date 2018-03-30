/* include file*/
#include <functional>
#include <vector>
using namespace std;

template <class Monoid>
struct LazyOperator {
  using Func = function<Monoid(Monoid, Monoid)>;

  Func node_update;
  function<Monoid(int, int, Monoid, Monoid)> lazy_make;
  Func lazy_effect;
  Func lazy_throw;
};

template <class Monoid>
struct LazySegment {
  using Func = function<Monoid(Monoid, Monoid)>;
  vector<Monoid> node;
  vector<Monoid> lazy;
  vector<bool> lazyFlag;
  int n;
  Monoid ide;
  Monoid lazy_init;

  LazyOperator<Monoid> ope;

  LazySegment(const vector<Monoid> init, Monoid ide_, Monoid lazy_i,
              LazyOperator<Monoid> ope_)
      : ope(ope_), ide(ide_), lazy_init(lazy_i) {
    n = 1;
    int sz = init.size();
    while (n < sz) n *= 2;

    node.resize(n * 2 - 1, ide);
    lazy.resize(n * 2 - 1, lazy_init);
    lazyFlag.resize(n * 2 - 1, false);

    for (int i = 0; i < sz; i++) node[i + n - 1] = init[i];
    for (int i = n - 2; i >= 0; i--)
      node[i] = ope.node_update(node[i * 2 + 1], node[i * 2 + 2]);
  }

  void eval(int k, int l, int r) {
    if (lazyFlag[k]) {
      node[k] = ope.lazy_effect(node[k], lazy[k]);

      if (r - l > 1) {
        lazy[2 * k + 1] = ope.lazy_throw(lazy[k], lazy[2 * k + 1]);
        lazy[2 * k + 2] = ope.lazy_throw(lazy[k], lazy[2 * k + 2]);
        lazyFlag[2 * k + 1] = true;
        lazyFlag[2 * k + 2] = true;
      }

      lazyFlag[k] = false;
      lazy[k] = lazy_init;
    }
  }

  void update_inter(int a, int b, Monoid x, int k = 0, int l = 0, int r = -1) {
    if (r < 0) r = n;

    eval(k, l, r);

    if (r <= a || b <= l) return;

    if (a <= l && r <= b) {
      lazy[k] = ope.lazy_make(l, r, x, lazy[k]);
      lazyFlag[k] = true;
      eval(k, l, r);
    } else {
      update_inter(a, b, x, k * 2 + 1, l, (l + r) / 2);
      update_inter(a, b, x, k * 2 + 2, (l + r) / 2, r);
      node[k] = ope.node_update(node[k * 2 + 1], node[k * 2 + 2]);
    }
  }

  Monoid get_inter(int a, int b, int k = 0, int l = 0, int r = -1) {
    if (r < 0) r = n;

    eval(k, l, r);

    if (r <= a || b <= l) return ide;

    if (a <= l && r <= b) return node[k];

    Monoid lm = get_inter(a, b, k * 2 + 1, l, (l + r) / 2);
    Monoid rm = get_inter(a, b, k * 2 + 2, (l + r) / 2, r);
    return ope.node_update(lm, rm);
  }
};

/*
LazyOperator

node_update(Monoid left,Monoid right)
    親の頂点の値を求める

lazy_make(int l,int r,Monoid x,Monoid lazy)
    遅延評価させる値を作る時
    もとから[l,r]lazyという遅延があり,xを遅延させる時

lazy_effect(Monoid node,Monoid lazy)
    ノードを遅延評価する
LazyOperator

node_update(Monoid left,Monoid right)
    親の頂点の値を求める

lazy_make(int l,int r,Monoid x,Monoid lazy)
    遅延評価させる値を作る時
    もとから[l,r]lazyという遅延があり,xを遅延させる時

lazy_effect(Monoid node,Monoid lazy)
    ノードを遅延評価する

lazy_throw(Monoid gen , Monoid throw_lazy)
    genが投げる先、throw_lazyが投げる遅延
*/