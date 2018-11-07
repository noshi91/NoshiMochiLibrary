#include <vector>
#include <cassert>

template<class Info>
struct edge{
  int to;
  Info info;
  edge(int to , Info info) : to(to) , info(info){}
};

struct normal_edge{
  normal_edge(){}
};

template<class Info>
class graph{
protected:
  using edge_type = edge<Info>;
  ::std::size_t N;
  ::std::vector<::std::vector<edge_type>> edges;
public:
  graph(::std::size_t N_) : N(N_) , edges(N_){}
  virtual void add_edge(int x , int y , Info info) = 0;
  const ::std::vector<edge_type>& delta(int v) const{
    assert(v < N);
    return edges[v];
  }
  const ::std::vector<edge_type>& operator[](int v) const{
    return this->delta(v);
  }
  ::std::size_t size() const{
    return N;
  }
  
};

template<class Info>
class undirected_graph : public graph<Info>{
public:
  undirected_graph(::std::size_t N) : graph<Info>(N){}
  void add_edge(int x, int y , Info info){
    this->edges[x].push_back(edge<Info>(y , info));
    this->edges[y].push_back(edge<Info>(x , info));
  }
};

template<class Info>
class bipartite_graph : public undirected_graph<Info>{
  ::std::size_t A;
  ::std::size_t B;
public:
  bipartite_graph(::std::size_t A_ , ::std::size_t B_) : A(A_) , B(B_) , undirected_graph<Info>(A_ + B_) {
  }
  void add_edge(int x , int y , Info info){
    undirected_graph<Info>::add_edge(x , y + A , info);
  }
  int left_vertex(int id) const{return id;}
  int right_vertex(int id) const{return id - A;}
  ::std::size_t left_size() const{ return A; }
  ::std::size_t right_size() const{ return B; }
};

#include <set>
#include <functional>
#include <queue>

template<class Info>
::std::vector<::std::pair<int,int>> hopcroft_karp(const bipartite_graph<Info>& g){
  int N = g.size();
  ::std::vector<int> dist(N);
  ::std::vector<int> match(N,-1);
  ::std::vector<bool> used(N , false);
  ::std::vector<bool> vis;

  ::std::function<bool(int)> dfs = [&](int v){
    vis[v] = true;
    for(auto& e : g[v]){
      int m = match[e.to];
      if(m < 0 || (!vis[m] && dist[m] == dist[v] + 1 && dfs(m))){
        match[e.to] = v;
        used[v] = true;
        return true;
      }
    }
    return false;
  };

  while(true){
    vis.assign(N , false);
    {
      dist.assign(N , -1);
      ::std::queue<int> que;
      for(int i = 0;i < N;i++){
        if(!used[i]){
          que.push(i);
          dist[i] = 0;
        }
      }

      while(!que.empty()){
        int v = que.front();
        que.pop();
        for(auto& e : g[v]){
          int m = match[e.to];
          if(m >= 0 && dist[m] == -1){
            dist[m] = dist[v] + 1;
            que.push(m);
          }
        }
      }
    }
    bool END = true;
    for(int i = 0;i < g.left_size();i++){
      if(!used[i] && dfs(i)) END = false;
    }
    if(END) break;
  }
  ::std::vector<::std::pair<int,int>> ans;
  for(int i = g.left_size();i < g.size();i++){
    if(match[i] != -1) ans.emplace_back(i,g.right_vertex(match[i]));
  }
  return ans;
}

//verify https://soundhound2018.contest.atcoder.jp/submissions/3560142


#include <bits/stdc++.h>
using namespace std;
using i64 = long long;
#define rep(i,s,e) for(int (i) = (s);(i) <= (e);(i)++)
#define all(x) x.begin(),x.end()

string s[200];
int num[202][202];
int dx[] = {1,-1,0,0};
int dy[] = {0,0,1,-1};

int main(){
  int r,c;
  cin >> r >> c;
  int A = 0;
  int B = 0;
  for(int i = 0;i < r;i++) for(int j = 0;j < c;j++){
    if((i + j) & 1) num[i][j] = B++;
    else num[i][j] = A++;
  }
  
  bipartite_graph<normal_edge> g(A , B);
  int cnt = 0;
  for(int i = 0;i < r;i++){
    cin >> s[i];
    cnt += count(s[i].begin(),s[i].end(),'.');
  }

  for(int i = 0;i < r;i++) for(int j =0;j < c;j++){
    if(((i + j) & 1) || s[i][j] == '*') continue;
    for(int di = 0;di < 4;di++){
      int nx = i + dx[di];
      int ny = j + dy[di];
      if(0 <= nx and nx < r and 0 <= ny and ny < c and s[nx][ny] == '.'){
        g.add_edge(num[i][j], num[nx][ny] , normal_edge());
      }
    }
  }
  cout << cnt - (int) hopcroft_karp (g).size() << endl;
}

