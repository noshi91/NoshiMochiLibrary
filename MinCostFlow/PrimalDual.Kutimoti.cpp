#include <queue>
#include <set>
#include <vector>
using namespace std;
using i64 = long long;

struct Graph {
  struct edge {
    int to;
    i64 cap;
    i64 cost;
    int rev;
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

struct Primal_Dual {
  Graph g;
  const int n;
  Primal_Dual(int n_) : g(n_), n(n_) {}

  void add_edge(int from, int to, i64 cost, i64 cap, i64 revcap) {
    g[from].push_back({to, cap, cost, (int)g[to].size()});
    g[to].push_back({from, revcap, -cost, (int)g[from].size() - 1});
  }

  i64 minimumCostFlow(int s, int t, i64 f) {
    using Pi = pair<i64, i64>;
    priority_queue<Pi, vector<Pi>, greater<Pi>> que;
    vector<int> prevv(n, -1);
    vector<int> preve(n, -1);
    vector<i64> potential(n, 0);
    i64 res = 0;
    while (f > 0) {
      vector<i64> dist(n, 1e9);
      que.push({0, s});
      dist[s] = 0;
      while (!que.empty()) {
        Pi p = que.top();
        que.pop();
        if (dist[p.second] < p.first) continue;
        for (int i = 0; i < (int)g[p.second].size(); i++) {
          auto &e = g[p.second][i];
          i64 next =
              dist[p.second] + e.cost + potential[p.second] - potential[e.to];
          if (e.cap > 0 && dist[e.to] > next) {
            dist[e.to] = next;
            prevv[e.to] = p.second;
            preve[e.to] = i;
            que.push({dist[e.to], e.to});
          }
        }
      }
      if (dist[t] == 1e9) return -1;
      for (int v = 0; v < n; v++) {
        potential[v] += dist[v];
      }
      i64 d = f;
      for (int v = t; v != s; v = prevv[v]) {
        d = min(d, g[prevv[v]][preve[v]].cap);
      }
      f -= d;
      res += d * potential[t];

      for (int v = t; v != s; v = prevv[v]) {
        auto &e = g[prevv[v]][preve[v]];
        e.cap -= d;
        g[v][e.rev].cap += d;
      }
    }
    return res;
  }
};
