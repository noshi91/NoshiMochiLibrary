typedef long long i64;

i64 MOD = 1e9 + 7;
// Euclid
i64 inv_mod(i64 a, i64 m = MOD) {
  i64 b, x, u, q, abs_m, tmp;

  abs_m = (m < 0) ? -m : m;
  b = m;
  x = 1;
  u = 0;
  while (b > 0) {
    q = a / b;
    tmp = u;
    u = x - q * u;
    x = tmp;
    tmp = b;
    b = a - q * b;
    a = tmp;
  }

  return (x < 0) ? abs_m + x : x;
}