#include <unordered_map>
#include <vector>
#include <set>
#include <map>
#include <array>
#include <iostream>

#define getnode(x) bdd::V[x]
#define hash_pair(x, y) \
	((((size_t)(x)+(size_t)(y))*((size_t)(x)+(size_t)(y)+1)>>1)+(size_t)(y))

typedef int64_t int_t;
struct bdd;
typedef std::array<bdd, 3> ite_memo;
template<> struct std::hash<bdd> { size_t operator()(const bdd& b) const; };
template<> struct std::hash<ite_memo>{size_t operator()(const ite_memo&m)const;};

extern int_t T, F;

class bdd {
	bdd(){}
	bdd(size_t v, int_t h, int_t l);
	void decref();
	void rehash() { hash = hash_pair(hash_pair(v, h), l); }
	static std::unordered_map<bdd, int_t> M;
	static std::unordered_map<ite_memo, int_t> C;
	static std::set<int_t> G;
public:
	static std::vector<bdd> V;
	static bool onexit;
	size_t v, refs, hash;
	int_t h, l;
	bool operator==(const bdd& b) const {
		return hash == b.hash && v == b.v && h == b.h && l == b.l;
	}
	static void init();
	static bool leaf(int_t t) { return abs(t) == T; } //!V[abs(t)].v; }
	static bool trueleaf(int_t t) { return t > 0; }
	static int_t add(size_t v, int_t h, int_t l);
	static int_t from_bit(size_t b, bool v);
	static std::wostream& out(std::wostream& os, int_t x);
	static void gc();
	~bdd();
};

int_t bdd_and(int_t x, int_t y);
