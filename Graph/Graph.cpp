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
private:
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
    this->edges[x].push_back(this->edge_type(y , info));
    this->edges[y].push_back(this->edge_type(x , info));
  }
};

template<class Info>
class directed_graph : public graph<Info>{
public:
  directed_graph(::std::size_t N) : graph<Info>(N){}
  void add_edge(int x, int y , Info info){
    this->edges[x].push_back(this->edge_type(y , info));
  }
};
