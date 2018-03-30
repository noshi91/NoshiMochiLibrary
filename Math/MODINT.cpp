#include<cstdint>

template<std::uint_fast32_t MODULO>
struct modint {
private:
	using uint32 = std::uint_fast32_t;
	using uint64 = std::uint_fast64_t;

public:
	uint32 a;
	modint() :a(0) {}
	modint(const std::int_fast64_t &x) :a(set(x%MODULO + MODULO)) {}
	static uint32 set(const uint32 &x) { return(x<MODULO) ? x : x - MODULO; }
	static modint make(const uint32 &x) { modint ret;ret.a = x;return ret; }
	modint operator+(const modint &o)const { return make(set(a + o.a)); }
	modint operator-(const modint &o)const { return make(set(a + MODULO - o.a)); }
	modint operator*(const modint &o)const { return make((uint64)a*o.a%MODULO); }
	modint operator/(const modint &o)const { return make((uint64)a*~o%MODULO); }
	modint &operator+=(const modint &o) { return *this = *this + o; }
	modint &operator-=(const modint &o) { return *this = *this - o; }
	modint &operator*=(const modint &o) { return *this = *this * o; }
	modint &operator/=(const modint &o) { return *this = *this / o; }
	modint &operator^=(const uint32 &o) { return *this = *this^o; }
	modint operator~ ()const { return *this ^ (MODULO - 2); }
	modint operator- ()const { return make(set(MODULO - a)); }
	modint operator++() { return *this = make(set(a + 1)); }
	modint operator--() { return *this = make(set(a + MODULO - 1)); }
	bool operator==(const modint &o)const { return a == o.a; }
	bool operator!=(const modint &o)const { return a != o.a; }
	bool operator< (const modint &o)const { return a <  o.a; }
	bool operator<=(const modint &o)const { return a <= o.a; }
	bool operator> (const modint &o)const { return a >  o.a; }
	bool operator>=(const modint &o)const { return a >= o.a; }
	explicit operator bool()const { return a; }
	explicit operator uint32()const { return a; }
	modint operator^(uint32 x)const {
		uint64 t = a, u = 1;
		while (x) { if (x & 1) u = u*t%MODULO;t = (t*t) % MODULO;x >>= 1; }
		return make(u);
	}
};

/*

verify:https://beta.atcoder.jp/contests/arc059/submissions/2280668

template<std::uint_fast32_t MODULO>
struct modint;

-説明略-


使い方

using mint = modint<1000000007>;
mint x;
std::cin >> x.a ;
std::cout<< x.a ;

※MODULOが合成数だと除算がバグります


整数型と異なる意味の演算子
-operator^
 累乗

-operator~
 逆元

*/