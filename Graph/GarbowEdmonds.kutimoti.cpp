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
  undirected_graph(::std::size_t N_) : graph<Info>(N_){}
  void add_edge(int x, int y , Info info){
    this->edges[x].push_back(edge<Info>(y , info));
    this->edges[y].push_back(edge<Info>(x , info));
  }
};

#include <set>
#include <queue>
#include <functional>
#include <algorithm>

struct labeled_edge{
  int label;
  labeled_edge(int l) : label(l){}
};

template<class Info>
::std::vector<::std::pair<int,int>> garbow_edmonds(const undirected_graph<Info>& gra){
  int N = gra.size();
  undirected_graph<labeled_edge> g(N + 1);
  ::std::vector<::std::pair<int,int>> edges;
  {
    int cnt = N + 1;
    for(int i = 0;i < N;i++){
      for(auto e : gra[i]){
        if(i < e.to){
          g.add_edge(i + 1, e.to + 1, labeled_edge(cnt++));
          edges.push_back({i + 1, e.to + 1});
        }
      }
    }
  }
  ::std::vector<int> mate(N + 1 , 0);
  ::std::vector<int> label(N + 1 , -1);
  ::std::vector<int> first(N + 1 , 0);

  ::std::function<int(int)> eval_first = [&](int x){
    if(label[first[x]] < 0) return first[x];
    first[x] = eval_first(first[x]);
    return first[x];
  };

  ::std::function<void(int,int)> rematch = [&](int v,int w){
    int t = mate[v];
    mate[v] = w;
    if(mate[t] != v) return;
    if(label[v] <= N){
      mate[t] = label[v];
      rematch(label[v] , t);
    }
    else{
      int x = edges[label[v] - N + 1].first;
      int y = edges[label[v] - N + 1].second;
      rematch(x , y);
      rematch(y , x);
    }
  };

  ::std::function<void(int,int,int)> assignLabel = [&](int x, int y,int num){
    int r = eval_first(x);
    int s = eval_first(y);
    int join = 0;
    if(r == s) return;
    label[r] = -num;
    label[s] = -num;
    while(true){
      if(s != 0) ::std::swap(r , s);
      r = eval_first(label[mate[r]]);
      if(label[r] == -num){
        join = r;
        break;
      }
      label[r] = -num;
    }
    int v = first[x];
    while(v != join){
      label[v] = num;
      first[v] = join;
      v = first[label[mate[v]]];
    }
    v = first[y];
    while(v != join){
      label[v] = num;
      first[v] = join;
      v = first[label[mate[v]]];
    }
    return;
  };

  ::std::function<bool(int)> augment_check = [&](int u){
    first[u] = 0;
    label[u] = 0;
    ::std::queue<int> que;
    que.push(u);
    while(!que.empty()){
      int x = que.front();
      que.pop();
      for(auto e : g[x]){
        int y = e.to;
        if(mate[y] == 0 && y != u){
          mate[y] = x;
          rematch(x , y);
          return true;
        }
        else if(label[y] >= 0){
          assignLabel(x , y , e.info.label);
        }
        else if(label[mate[y]] < 0){
          label[mate[y]] = x;
          first[mate[y]] = y;
          que.push(mate[y]);
        }
      }
    }
    return false;
  };

  for(int i = 1;i <= N;i++){
    if(mate[i] != 0) continue;
    if(augment_check(i)){
      ::std::fill(label.begin() , label.end() , -1);
    }
  }

  ::std::vector<::std::pair<int,int>> ans;
  for(int i = 1;i <= N;i++){
    if(i < mate[i]){
      ans.push_back({i , mate[i]});
    }
  }
  return ans;
}

/*
 * kutimoti/nim-algorithm-libraryのC++移植です.詳しくはhttps://kutimoti.github.io/library/GarbowsEdmonds.htmlをどうぞ
 */

//verify https://soundhound2018.contest.atcoder.jp/submissions/3555429

#include <bits/stdc++.h>
using namespace std;
using i64 = long long;
#define rep(i,s,e) for(int (i) = (s);(i) <= (e);(i)++)
#define all(x) x.begin(),x.end()

string s[200];
int dx[] = {1,-1,0,0};
int dy[] = {0,0,1,-1};

int main(){
  int r,c;
  cin >> r >> c;
  undirected_graph<normal_edge> g(r * c);
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
        g.add_edge(i * c + j , nx * c + ny , normal_edge());
      }
    }
  }

  cout << cnt - (int)garbow_edmonds(g).size() << endl;
}
