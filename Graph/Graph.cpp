#include <vector>
#include <cassert>

template<class id_type = size_t>
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
  Graph(size_t sz) : ver(sz) , edges(sz) , G(sz) , rG(sz){
    for(size_t i = 0;i < ver.size();i++) ver[i] = i;
  }

  const std::vector<vertex_type>& V() const{
    return ver;
  }

  const std::vector<Edge_Pair>& E() const{
    return edges;
  }

  void add_edge(const Edge& e){
    edges.push_back(e);
    G[e.from].push_back({e.to,e.id});
    G[e.to].push_back({e.from,e.id});
  }

  void add_edge(vertex_type from,vertex_type to,id_type id){
    add_edge({from,to,id});
  }

  Graph operator+=(const Edge& e){
    add_edge(e);
    return *this;
  }


  const std::vector<Edge>& delta_p(vertex_type v){
    return G[v];
  }

  const std::vector<Edge>& delta_m(vertex_type v){
    return rG[v];
  }

  const std::vector<Edge>& operator[](vertex_type v){
    return this->delta_p(v);
  }
};
