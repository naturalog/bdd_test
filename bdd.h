#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>
#include <map>
#include <array>
#include <iostream>
#include <memory>

#define getnode(x) bdd::V[x]
#define hash_pair(x, y) \
	((((size_t)(x)+(size_t)(y))*((size_t)(x)+(size_t)(y)+1)>>1)+(size_t)(y))
#define hash_tri(x, y, z) hash_pair(hash_pair(x, y), z)
#define has(x, y) ((x).find(y) != (x).end())
#define hasb(x, y) std::binary_search(x.begin(), x.end(), y)

typedef int64_t int_t;
struct bdd;
typedef std::vector<int_t> bdds;
typedef std::array<int_t, 3> ite_memo;
template<> struct std::hash<std::tuple<size_t, size_t, int_t, int_t>> {
	size_t operator()(const std::tuple<size_t,size_t,int_t,int_t>& k) const{
		return std::get<0>(k);
	}
};
template<> struct std::hash<ite_memo>{size_t operator()(const ite_memo&m)const;};
template<> struct std::hash<bdds> { size_t operator()(const bdds&) const; };

extern int_t T, F;

class bdd {
	void rehash() { hash = hash_tri(v, h, l); }
	typedef std::tuple<size_t, size_t, int_t, int_t> key;
	static std::unordered_map<key, int_t> M;
	static std::unordered_map<ite_memo, int_t> C;
	static std::unordered_map<bdds, int_t> AM;
	static std::unordered_set<int_t> S;
	static void mark_all(int_t i);
	static size_t bdd_and_many_iter(bdds, bdds&, bdds&, int_t&, size_t&);
	int_t h, l;
public:
	bdd(){}
	bdd(size_t v, int_t h, int_t l);
	static std::vector<bdd> V;
	static bool onexit;
	size_t v, /*refs,*/ hash;
	key getkey() const { return { hash, v, h, l }; }
	inline bool operator==(const bdd& b) const {
		return hash == b.hash && v == b.v && h == b.h && l == b.l;
	}
	static void init();
	static void mark(int_t i) { S.insert(abs(i)); }
	static void unmark(int_t i) { S.erase(abs(i)); }
	inline static bool leaf(int_t t) { return abs(t) == T; }
	inline static bool trueleaf(int_t t) { return t > 0; }
	inline static int_t add(size_t v, int_t h, int_t l);
	inline static int_t from_bit(size_t b, bool v);
	static std::wostream& out(std::wostream& os, int_t x);
	static void gc();
	inline static int_t hi(int_t x) { return x > 0 ? V[x].h : -V[-x].h; }
	inline static int_t lo(int_t x) { return x > 0 ? V[x].l : -V[-x].l; }
	inline static size_t var(int_t x) { return x > 0 ? V[x].v : -V[-x].v; }
	static int_t bdd_and(int_t x, int_t y);
	static int_t bdd_or(int_t x, int_t y) { return -bdd_and(-x, -y); }
	static int_t bdd_ite(int_t x, int_t y, int_t z);
	static int_t bdd_and_many(bdds v);
};

typedef std::shared_ptr<class bdd_handle> spbdd_handle;

class bdd_handle {
	bdd_handle(int_t b) : b(b) { bdd::mark(b); }
public:
	int_t b;
	static std::unordered_map<int_t, std::weak_ptr<bdd_handle>> M;
	static void update(const std::vector<int_t>&,
		const std::unordered_set<int_t>&);
	static spbdd_handle get(int_t b) {
		auto it = M.find(b);
		if (it != M.end()) return it->second.lock();
		spbdd_handle h(new bdd_handle(b));
		return	M.emplace(b, std::weak_ptr<bdd_handle>(h)), h;
	}
	~bdd_handle() { if (abs(b) > 1) bdd::unmark(b); M.erase(b); }
};
