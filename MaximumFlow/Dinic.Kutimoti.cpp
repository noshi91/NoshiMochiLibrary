#include <iostream>
#include <queue>
#include <vector>
using namespace std;

using i64 = long long;

struct Graph {
  struct edge {
    int to;
    i64 cap;
    i64 rev;
  };

  int n;
  vector<vector<edge>> edges;

  Graph(int N) {
    n = N;
    edges.resize(n, vector<edge>());
  }

  int size() const { return n; }

  vector<edge> &operator[](int v) { return edges[v]; }
};

struct Dinic {
  int N;
  vector<int> level;
  vector<int> itr;
  Graph G;

  Dinic(int n) : G(n) { N = n; }

  void add_edge(int from, int to, i64 cap, i64 rev_cap) {
    G[from].push_back({to, cap, (int)G[to].size()});
    G[to].push_back({from, rev_cap, (int)G[from].size() - 1});
  }

  bool g_level(int s, int t) {
    level.assign(N, -1);
    queue<int> que;
    que.push(s);
    level[s] = 0;
    while (!que.empty()) {
      int v = que.front();
      que.pop();
      for (auto &e : G[v]) {
        if (e.cap > 0 && level[e.to] == -1) {
          level[e.to] = level[v] + 1;
          que.push(e.to);
        }
      }
    }
    return level[t] >= 0;
  }

  i64 dfs(int v, int t, i64 f) {
    if (v == t) return f;

    for (int &i = itr[v]; i < G[v].size(); i++) {
      auto &e = G[v][i];
      if (e.cap > 0 && level[e.to] > level[v]) {
        i64 mi_f = dfs(e.to, t, min(f, e.cap));
        if (mi_f > 0) {
          e.cap -= mi_f;
          G[e.to][e.rev].cap += mi_f;
          return mi_f;
        }
      }
    }
    return 0;
  }

  i64 max_flow(int s, int t) {
    i64 result = 0;
    i64 flow;
    while (g_level(s, t)) {
      itr.assign(N, 0);
      while ((flow = dfs(s, t, 1e9)) > 0) result += flow;
    }
    return result;
  }
};

/*

checked
    http://judge.u-aizu.ac.jp/onlinejudge/review.jsp?rid=2723921#1
*/
