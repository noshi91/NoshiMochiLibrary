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
class directed_graph : public graph<Info>{
public:
  directed_graph(::std::size_t N) : graph<Info>(N){}
  void add_edge(int x, int y , Info info){
    this->edges[x].push_back(edge<Info>(y , info));
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
};
