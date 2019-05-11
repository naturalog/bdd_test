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

typedef int32_t int_t;
typedef uint32_t uint_t;
struct bdd;
typedef std::shared_ptr<class bdd_handle> spbdd_handle;
typedef const spbdd_handle& cr_spbdd_handle;
typedef std::vector<int_t> bdds;
typedef std::vector<spbdd_handle> bdd_handles;
typedef std::array<int_t, 3> ite_memo;
typedef std::vector<bool> bools;
typedef std::vector<bools> vbools;

template<> struct std::hash<std::tuple<uint_t, uint_t, int_t, int_t>> {
	size_t operator()(const std::tuple<uint_t,uint_t,int_t,int_t>& k) const;
};
template<> struct std::hash<ite_memo>{size_t operator()(const ite_memo&m)const;};
template<> struct std::hash<bdds> { size_t operator()(const bdds&) const; };

extern int_t T, F;

spbdd_handle from_bit(uint_t b, bool v);
bool leaf(cr_spbdd_handle h);
bool trueleaf(cr_spbdd_handle h);
std::wostream& out(std::wostream& os, cr_spbdd_handle x);
spbdd_handle bdd_and(cr_spbdd_handle x, cr_spbdd_handle y);
spbdd_handle bdd_or(cr_spbdd_handle x, cr_spbdd_handle y);
spbdd_handle bdd_ite(cr_spbdd_handle x, cr_spbdd_handle y, cr_spbdd_handle z);
spbdd_handle bdd_and_many(const bdd_handles& v);
vbools allsat(cr_spbdd_handle x, uint_t nvars);

class bdd {
	friend class bdd_handle;
	friend spbdd_handle bdd_and(cr_spbdd_handle x, cr_spbdd_handle y);
	friend spbdd_handle bdd_or(cr_spbdd_handle x, cr_spbdd_handle y);
	friend spbdd_handle bdd_ite(cr_spbdd_handle x, cr_spbdd_handle y,
		cr_spbdd_handle z);
	friend spbdd_handle bdd_and_many(const bdd_handles& v);
	friend vbools allsat(cr_spbdd_handle x, uint_t nvars);
	friend spbdd_handle from_bit(uint_t b, bool v);
	friend bool leaf(cr_spbdd_handle h);
	friend bool trueleaf(cr_spbdd_handle h);
	friend std::wostream& out(std::wostream& os, cr_spbdd_handle x);
	void rehash() { hash = hash_tri(v, h, l); }
	typedef std::tuple<uint_t, uint_t, int_t, int_t> key;
	static std::unordered_map<key, int_t> M;
	static std::unordered_map<ite_memo, int_t> C;
	static std::unordered_map<bdds, int_t> AM;
	static std::unordered_set<int_t> S;
	static void mark_all(int_t i);
	static size_t bdd_and_many_iter(bdds, bdds&, bdds&, int_t&, size_t&);
	static void sat(uint_t v, uint_t nvars, int_t t, bools& p, vbools& r);
	static vbools allsat(int_t x, uint_t nvars);
	inline static int_t hi(int_t x) { return x > 0 ? V[x].h : -V[-x].h; }
	inline static int_t lo(int_t x) { return x > 0 ? V[x].l : -V[-x].l; }
	inline static uint_t var(int_t x) { return x > 0 ? V[x].v : -V[-x].v; }
	static int_t bdd_and(int_t x, int_t y);
	static int_t bdd_or(int_t x, int_t y) { return -bdd_and(-x, -y); }
	static int_t bdd_ite(int_t x, int_t y, int_t z);
	static int_t bdd_and_many(bdds v);
	static void mark(int_t i) { S.insert(abs(i)); }
	static void unmark(int_t i) { S.erase(abs(i)); }
	inline static int_t add(uint_t v, int_t h, int_t l);
	inline static int_t from_bit(uint_t b, bool v);
	inline static bool leaf(int_t t) { return abs(t) == T; }
	inline static bool trueleaf(int_t t) { return t > 0; }
	static std::wostream& out(std::wostream& os, int_t x);
	int_t h, l;
public:
	bdd(){}
	bdd(uint_t v, int_t h, int_t l);
	static std::vector<bdd> V;
	uint_t v, hash;
	key getkey() const { return { hash, v, h, l }; }
	inline bool operator==(const bdd& b) const {
		return hash == b.hash && v == b.v && h == b.h && l == b.l;
	}
	static void init();
	static void gc();
};

class bdd_handle {
	friend class bdd;
	bdd_handle(int_t b) : b(b) { bdd::mark(b); }
	static void update(const std::vector<int_t>&,
		const std::unordered_set<int_t>&);
	static std::unordered_map<int_t, std::weak_ptr<bdd_handle>> M;
public:
	int_t b;

	static spbdd_handle get(int_t b);
	static spbdd_handle get(uint_t v, cr_spbdd_handle h, cr_spbdd_handle l);
	static spbdd_handle T, F;
	~bdd_handle() { if (abs(b) > 1) bdd::unmark(b); M.erase(b); }
};
