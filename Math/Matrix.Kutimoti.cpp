#include <iostream>
#include <vector>
using namespace std;

template <class T>
struct Matrix {
  vector<vector<T>> value;
  T init;

  Matrix(vector<vector<T>> vec, T init_) : init(init_), value(vec) {}
  Matrix(int h, int w, T init_) : init(init_), value(h, vector<T>(w, init_)) {}

  Matrix<T> operator*(Matrix<T>& left) {
    Matrix<T> res(value.size(), left[0].size(), init);
    for (int i = 0; i < value.size(); i++) {
      for (int k = 0; k < left.size(); k++) {
        for (int j = 0; j < left[0].size(); j++) {
          res[i][j] = (res[i][j] + value[i][k] * left[k][j]);
        }
      }
    }
    return res;
  }

  Matrix<T> operator+(Matrix<T>& left) {
    Matrix<T> res = *this;
    for (int i = 0; i < res.size(); i++) {
      for (int j = 0; j < res[0].size(); j++) {
        res[i][j] += left[i][j];
      }
    }
    return res;
  }

  Matrix<T> operator*(T c) {
    Matrix<T> res = *this;
    for (int i = 0; i < res.size(); i++) {
      for (int j = 0; j < res[0].size(); j++) {
        res[i][j] *= c;
      }
    }
    return res;
  }

  vector<T>& operator[](int n) { return value[n]; }

  size_t size() { return value.size(); }

  Matrix<T> operator^(long long n) {
    Matrix<T> a = *this;
    Matrix<T> res(a.size(), a.size(), init);
    for (int i = 0; i < a.size(); i++) {
      res[i][i] = 1;
    }

    while (n > 0) {
      if (n & 1) res = res * a;
      a = a * a;
      n >>= 1;
    }
    return res;
  }
};
