#include <unordered_map>
#include <vector>
#include <set>
#include <map>
#include <array>

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
	static void init() {
		bdd b;
		b.v = 0, b.h = b.l = 0;
		V.push_back(b); // dummy
		F = -(T = add(0, 0, 0));
	}
	static bool leaf(size_t t) { return !V[t].v; }
	static bool trueleaf(size_t t) { return t > 0; }
	static int_t add(size_t v, int_t h, int_t l) {
		if (h == l) return h;
		if (l < 0) {
			bdd b(v, -h, -l);
			auto it = M.find(b);
			if (it != M.end()) return it->second;
			return V.push_back(std::move(b)),
			       M.emplace(V.back(), V.size() - 1), -V.size()+1;
		}
		bdd b(v, h, l);
		auto it = M.find(b);
		if (it != M.end()) return it->second;
		return	V.push_back(std::move(b)),
			M.emplace(V.back(), V.size() - 1), V.size()-1;
	}
	static int_t from_bit(size_t b, bool v) {
		return v ? add(b+1, T, F) : add(b+1, F, T);
	}
	static void gc();
	~bdd();
};

int_t bdd_and(int_t x, int_t y);
std::vector<std::vector<bool>> allsat(int_t x, size_t nvars);
