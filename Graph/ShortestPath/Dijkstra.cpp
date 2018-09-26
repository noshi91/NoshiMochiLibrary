#include <vector>
#include <cassert>

using size_t = std::size_t;

template<class id_type>
class Graph{
public:
  using vertex_type = size_t;
  struct Edge{
    vertex_type to;
    id_type id;
  };

  struct Edge_Pair{
    vertex_type from;
    vertex_type to;
    id_type id;
  };
private:
  std::vector<vertex_type> ver;
  std::vector<Edge_Pair> edges;
  std::vector<std::vector<Edge>> G;
  std::vector<std::vector<Edge>> rG;
public:
  Graph(size_t sz) : ver(sz) , G(sz) , rG(sz){
    for(size_t i = 0;i < ver.size();i++) ver[i] = i;
  }

  const std::vector<vertex_type>& V() const{
    return ver;
  }

  const std::vector<Edge_Pair>& E() const{
    return edges;
  }

  void add_edge(const Edge_Pair& e){
    edges.push_back(e);
    G[e.from].push_back({e.to,e.id});
    rG[e.to].push_back({e.from,e.id});
  }

  void add_edge(vertex_type from,vertex_type to,id_type id){
    add_edge({from,to,id});
  }

  Graph operator+=(const Edge_Pair& e){
    add_edge(e);
    return *this;
  }


  const std::vector<Edge>& delta_p(vertex_type v) const{
    return G[v];
  }

  const std::vector<Edge>& delta_m(vertex_type v) const{
    return rG[v];
  }

  const std::vector<Edge>& operator[](vertex_type v) const{
    return this->delta_p(v);
  }
};

#include <set>
#include <queue>
#include <functional>

template<class dist_type,class id_type>
std::vector<dist_type> dijkstra(const Graph<id_type>& g,std::function<dist_type(id_type)> cost,size_t s,dist_type init,dist_type INF){
    using vt = typename Graph<id_type>::vertex_type;
    using P = std::pair<dist_type,vt>;
    std::vector<dist_type> dist(g.V().size(),INF);
    dist[s] = init;

    std::priority_queue<P,std::vector<P>,std::greater<P>> que;

    que.push({dist[s],s});

    while(!que.empty()){
      vt v = que.top().second;
      dist_type d = que.top().first;
      que.pop();
      if(dist[v] < d) continue;
      for(const auto & e : g.delta_p(v)){
        if(dist[e.to] > dist[v] + cost(e.id)){
          dist[e.to] = dist[v] + cost(e.id);
          que.push({dist[e.to],e.to});
        }
      }
    }
    return dist;
}
