#include <string>
#include <vector>
using i64 = long long;
using namespace std;

const vector<i64> RHMOD = {999999937LL, 1000000007LL};

struct Rolling_Hash {
  int n;
  i64 base;
  vector<vector<i64>> hs, pw;
  Rolling_Hash() {}
  Rolling_Hash(const string& s, i64 base_ = 9973)
      : n(s.size()), base(base_), hs(RHMOD.size()), pw(RHMOD.size()) {
    for (int i = 0; i < RHMOD.size(); i++) {
      hs[i].assign(n + 1, 0);
      pw[i].assign(n + 1, 0);
      hs[i][0] = 0;
      pw[i][0] = 1;
      for (int j = 0; j < n; j++) {
        pw[i][j + 1] = pw[i][j] * base % RHMOD[i];
        hs[i][j + 1] = (hs[i][j] * base + s[j]) % RHMOD[i];
      }
    }
  }

  i64 hash(int l, int r, int i) {
    return ((hs[i][r] - hs[i][l] * pw[i][r - l]) % RHMOD[i] + RHMOD[i]) %
           RHMOD[i];
  }

  i64 match(int l1, int r1, int l2, int r2) {
    bool res = true;
    for (int i = 0; i < RHMOD.size(); i++)
      res &= (hash(l1, r1, i) == hash(l2, r2, i));
    return res;
  }
};
