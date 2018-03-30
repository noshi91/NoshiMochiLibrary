#include <vector>
using namespace std;


template <class Monoid>
struct BIT_2D{
    vector<vector<Monoid>> bit;
    //identity element
    Monoid ide;
    //bit.size()
    int N;
    //bit[i].size()
    int M;

    BIT_2D(int n,int m,Monoid ide) : N(n) , M(m) ,ide(ide), bit(N,vector<Monoid>(M,ide)){}

    void add(int r,int c,Monoid value){
        for(int i = r;i < N;i |= i + 1){
            for(int j = c;j < M;j |= j + 1){
                bit[i][j] += value;
            }
        }
    }
    
    
    //sum[(0,0),(r,c)]
    Monoid sum(int r,int c){
        int res = 0;
        for(int i = r;i >= 0;i = (i & (i + 1)) - 1){
            for(int j = c;j >= 0;j = (j & (j + 1)) - 1){
                res += bit[i][j];
            }
        }
        return res;
    }
    //sum[(r1,c1),(r2,c2)]
    Monoid sum(int r1,int c1,int r2,int c2){
        return sum(r2,c2) - sum(r1 - 1,c2) - sum(r2,c1 - 1) + sum(r1 - 1 , c1 - 1);
    }

    Monoid get(int r,int c){
        return sum(r,c,r,c);
    }

    void set(int r,int c,Monoid value){
        add(r,c,value - get(r,c));
    }
};

/*
checked : http://judge.u-aizu.ac.jp/onlinejudge/review.jsp?rid=2756697#1
*/