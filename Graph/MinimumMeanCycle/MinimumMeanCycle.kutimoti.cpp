#include <vector>
#include <tuple>
#include <utility>
using namespace std;

class MinimumMeanCycle{
public:
  struct Edge{
    int to;
    double cost;
  };
private:
  vector<vector<Edge>> rG;
  const int n;
  const int s;
  const double INF = 1e9;

public:
  MinimumMeanCycle(int sz) : n(sz + 1),rG(sz + 1),s(sz){
    for(int i = 0;i < n - 1;i++){
      this->add_edge(s,i,0);
    }
  }

  void add_edge(int from,int to,double cost){
    rG[to].push_back({from,cost});
  }

  tuple<bool,double> mmc(){

    vector<vector<double>> F(n + 1,vector<double>(n + 1,INF));
    //vector<vector<int>> p(n,vector<int>(n + 1));
    F[0][s] = 0;
    for(int k = 1;k <= n;k++){
      for(int x = 0;x < n;x++){
        F[k][x] = INF;
        for(auto & re : rG[x]){
          if(F[k][x] > F[k - 1][re.to] + re.cost){
            F[k][x] = F[k - 1][re.to] + re.cost;
            //p[k][x] = re.to
          }
        }
      }
    }
    double ans = INF;
    for(int x = 0;x < n;x++){
      if(F[n][x] == INF) continue;
      double ret = 0;
      for(int k = 0;k < n;k++){
        if(F[k][x] == INF) continue;
        ret = max(ret , (F[n][x] - F[k][x]) / (n - k));
      }
      ans = min(ret , ans);
    }

    return {true,ans};
  }
};
#include <iostream>

int main(){
  int N,M;
  cin >> N >> M;
  MinimumMeanCycle mmc(N);
  for(int i = 0;i < M;i++){
    int a,b;
    double c;
    cin >> a >> b >> c;
    mmc.add_edge(a,b,c);
  }
  bool a;
  double b;
  tie(a,b) = mmc.mmc();
  cout << a << " " << b << endl;
}
