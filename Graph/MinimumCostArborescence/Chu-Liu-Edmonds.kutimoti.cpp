/**
 *
 * Chu-Liu/Edmonds's Algorithm
 *
 * 最小重み根指定有向木の総コスト
 *
 * O(|E||V|)
 *
 * http://judge.u-aizu.ac.jp/onlinejudge/review.jsp?rid=2875014#1
 *
 */

#include <vector>
#include <set>
using namespace std;
using INT = int;
using edge = pair<INT,pair<int,int>>;

INT INF = 1 << 30;

INT Edmonds(const vector<edge>& G,int V,int r){
  vector<pair<INT,int>> MIN(V,pair<INT,int>(INF,-1));
  for(auto &e : G){
    MIN[e.second.second] = min(MIN[e.second.second] , pair<INT,int>(e.first,e.second.first));
  }

  MIN[r] = {-1,-1};

  vector<int> group(V,0);
  vector<bool> isCycle(V,false);
  int count = 0;

  vector<bool> used(V,false);

  for(int i = 0;i < V;i++){
    if(used[i]) continue;
    vector<int> chain;
    int now = i;
    while(now != -1 && !used[now]){
      used[now] = true;
      chain.push_back(now);
      now = MIN[now].second;
    }
    if(now != -1){
      bool inCycle = false;
      for(int j : chain){
        group[j] = count;
        if(j == now){
          isCycle[count] = true;
          inCycle = true;
        }
        if(!inCycle) 
          count++;
      }
      if(inCycle) 
        count++;
    }
    else{
      for(int j : chain){
        group[j] = count;
        count++;
      }
    }
  }
  INT res = 0;
  if(count == V){
    for(int i = 0;i < V;i++){
      if(i == r) continue;
      res += MIN[i].first;
    }
    return res;
  }

  for(int i = 0;i < V;i++){
    if(i == r) continue;
    if(isCycle[group[i]])
      res += MIN[i].first;
  }

  vector<edge> nG;
  for(auto & e : G){
    int to = e.second.second;
    int gfrom = group[e.second.first];
    int gto = group[e.second.second];
    if(gfrom == gto) continue;
    else if(isCycle[gto])
      nG.push_back({e.first - MIN[to].first,{gfrom,gto}});
    else{
      nG.push_back({e.first,{gfrom,gto}});
    }
  }
  return res + Edmonds(nG,count,group[r]);
}
