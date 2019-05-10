#include <cassert>
#include <algorithm>
#include "bdd.h"
using namespace std;

int_t T, F;
unordered_map<bdd::key, int_t> bdd::M;
vector<bdd> bdd::V;
unordered_map<ite_memo, int_t> bdd::C;
unordered_set<int_t> bdd::S;
bool bdd::onexit = false;
typedef vector<bool> bools;
typedef vector<bools> vbools;
const size_t gclimit = 1000000;

void bdd::init() {
	V.emplace_back(0, 0, 0), // dummy
	//V.back().refs = -1,
	V.back().rehash(), T = 1, F = -1,
	V.emplace_back(0, 1, 1), //V.back().refs = -1,
	V.back().rehash(), M.emplace(V.back().getkey(), 1);
	mark(0), mark(T), mark(F);
}

int_t bdd::add(size_t v, int_t h, int_t l) {
	assert(v && h && l);
	if (h == l) return h;
	if (l < 0) {
		key k = { hash_tri(v, -h, -l), v, -h, -l };
		auto it = M.find(k);
		if (it != M.end()) return -it->second;
		onexit = true;
		V.emplace_back(v, -h, -l), M.emplace(k, V.size() - 1);
		onexit = false;
		return -V.size() + 1;
	}
	key k = { hash_tri(v, h, l), v, h, l };
	auto it = M.find(k);
	if (it != M.end()) return it->second;
	onexit = true;
	V.emplace_back(v, h, l), M.emplace(k, V.size() - 1);
	onexit = false;
	return V.size() - 1;
}

int_t bdd::from_bit(size_t b, bool v) {
	return v ? add(b + 1, T, F) : add(b + 1, F, T);
}

int_t bdd::bdd_and(int_t x, int_t y) {
	assert(x && y);
	if (x == F || y == F) return F;
	if (x == T || x == y) return y;
	if (y == T) return x;
	if (x == -y) return F;
	auto it = C.find({x,y,F});
	if (it != C.end()) return it->second;
	const size_t vx = V[abs(x)].v, vy = V[abs(y)].v;
	int_t r;
	if (vx < vy) r = add(vx, bdd_and(hi(x), y), bdd_and(lo(x), y));
	else if (vx > vy) r = add(vy, bdd_and(x, hi(y)), bdd_and(x, lo(y)));
	else r = add(vx, bdd_and(hi(x), hi(y)), bdd_and(lo(x), lo(y)));
	return C.emplace(ite_memo{x,y,F}, r), r;
}

int_t bdd::bdd_ite(int_t x, int_t y, int_t z) {
	assert(x && y && z);
	if (x == F) return z;
	if (x == T || y == z) return y;
	if (x == -y || x == z) return F;
	auto it = C.find({x, y, z});
	if (it != C.end()) return it->second;
	int_t r;
	const size_t vx = V[abs(x)].v, vy = V[abs(y)].v, vz = V[abs(z)].v;
	const size_t s = min(vx, min(vy, vz));
	if (y == T) r = bdd_or(x, z);
	else if (y == F) r = bdd_and(-x, z);
	else if (z == F) r = bdd_and(x, y);
	else if (z == T) r = bdd_or(-x, y);
	else if (vx == vy && vy == vz)
		r =	add(vx, bdd_ite(hi(x), hi(y), hi(z)),
				bdd_ite(lo(x), lo(y), lo(z)));
	else if (s == vx && s == vy)
		r =	add(vx, bdd_ite(hi(x), hi(y), z),
				bdd_ite(lo(x), lo(y), z));
	else if (s == vy && s == vz)
		r =	add(vy, bdd_ite(x, hi(y), hi(z)),
				bdd_ite(x, lo(y), lo(z)));
	else if (s == vx && s == vz)
		r =	add(vx, bdd_ite(hi(x), y, hi(z)),
				bdd_ite(lo(x), y, lo(z)));
	else if (s == vx)
		r =	add(vx, bdd_ite(hi(x), y, z), bdd_ite(lo(x), y, z));
	else if (s == vy)
		r =	add(vy, bdd_ite(x, hi(y), z), bdd_ite(x, lo(y), z));
	else	r =	add(vz, bdd_ite(x, y, hi(z)), bdd_ite(x, y, lo(z)));
	return C.emplace(ite_memo{x, y, z}, r), r;
}

void bdd::mark_all(int_t i) {
	if (S.find(i = abs(i)) != S.end()) return;
	S.insert(i), mark(hi(i)), mark(lo(i));
}

void bdd::gc() {
	if (V.size() - S.size() < 1000000) return;
	for (int_t i : S) mark_all(i);
	if (V.size() == S.size()) return;
	unordered_set<int_t> G;
	for (size_t n = 0; n != V.size(); ++n)
		if (S.find(n) == S.end())
			G.insert(n);
	assert(!G.empty());
	vector<int_t> p(V.size(), 0);
	for (size_t n = 0, k = 0; n != V.size(); ++n)
		if (G.find(n) != G.end())
			p[V.size() - ++k] = n, V[n] = V[V.size() - k];
	wcout << G.size() << ' ' << V.size() << endl;
	M.clear(), V.resize(V.size() - G.size());
#define f(i) (i = (i >= 0 ? p[i] ? p[i] : i : p[-i] ? -p[-i] : i))
	for (size_t n = 2; n != V.size(); ++n) {
//		assert(abs(V[n].h) != n); assert(abs(V[n].l) != n);
		f(V[n].h), f(V[n].l), V[n].rehash();
//		assert(abs(V[n].h) != n); assert(abs(V[n].l) != n);
	}
	unordered_map<ite_memo, int_t> c;
	for (pair<ite_memo, int_t> x : C)
		if (!(	has(G, abs(x.first[0])) ||
			has(G, abs(x.first[1])) ||
			has(G, abs(x.first[2])) ||
			has(G, abs(x.second))))
			f(x.first[0]), f(x.first[1]), f(x.first[2]),
			f(x.second), c.emplace(x.first, x.second);
#undef f
	wcout << c.size() << ' ' << C.size();
	C = move(c), G.clear();
	for (size_t n = 0; n < V.size(); ++n) M.emplace(V[n].getkey(), n);
	wcout << ' ' << V.size() << endl;
}

void sat(size_t v, size_t nvars, int_t t, bools& p, vbools& r) {
	if (bdd::leaf(t) && !bdd::trueleaf(t)) return;
	if (!bdd::leaf(t) && v < getnode(abs(t)).v)
		p[v - 1] = true, sat(v + 1, nvars, t, p, r),
		p[v - 1] = false, sat(v + 1, nvars, t, p, r);
	else if (v != nvars) {
		p[v - 1] = true, sat(v + 1, nvars, bdd::hi(t), p, r),
		p[v - 1] = false, sat(v + 1, nvars, bdd::lo(t), p, r);
	} else	r.push_back(p);
}

vbools allsat(int_t x, size_t nvars) {
	bools p(nvars);
	vbools r;
	return sat(1, nvars + 1, x, p, r), r;
}

size_t std::hash<ite_memo>::operator()(const ite_memo& m) const {
	return hash_pair(m[0], hash_pair(m[1], m[2]));
}

bdd::bdd(size_t v, int_t h, int_t l) : h(h), l(l), v(v) {//, refs(0) {
	rehash();
//	if (h && !leaf(h)) ++getnode(abs(h)).refs;
//	if (l && !leaf(l)) ++getnode(abs(l)).refs;
}

bdd::~bdd() {
//	if (onexit) return;
//	decref();//, getnode(abs(h)).decref(), getnode(abs(l)).decref();
}

wostream& bdd::out(wostream& os, int_t x) {
	if (leaf(x)) return os << (trueleaf(x) ? L'T' : L'F');
	if (x < 0) x = -x, os << L'~';
	const bdd& b = getnode(x);
	return out(out(os << b.v << L" ? ", b.h) << L" : ", b.l);
}

void bdd::decref() {
//	if (!v) return;
//	if (!refs || !--refs)
//		if (this >= &V[0] && this < &V[V.size()])
//			G.insert(this - &V[0]);
//	getnode(abs(h)).decref(), getnode(abs(l)).decref();
}

//void bdd::decref(int_t x) {
//	V[abs(x)].decref();
//	if (G.size() && (V.size() / G.size() <= 3)) gc();
//}

wostream& operator<<(wostream& os, const bools& x) {
	for (auto y : x) os << (y ? 1 : 0);
	return os;
}

wostream& operator<<(wostream& os, const vbools& x) {
	for (auto y : x) os << y << endl;
	return os;
}

int main() {
	bdd::init();
	for (size_t n = 0; n != 10000000; ++n) {
		const int_t x = bdd::from_bit(random()%10000+1, true);
		const int_t y = bdd::from_bit(random()%10000+1, false);
		const int_t z = bdd::from_bit(random()%10000+1, false);
		const int_t t = bdd::bdd_ite(x, y, z);
		if (random()&1) bdd::mark(t);
		bdd::gc();
	}
	const int_t x = bdd::from_bit(0, true);
	const int_t y = bdd::from_bit(1, false);
	int_t z = bdd::bdd_and(x, y);
	assert(bdd::from_bit(0, true) == -bdd::from_bit(0, false));
	assert(bdd::from_bit(3, true) == -bdd::from_bit(3, false));
	bdd::out(wcout, x) << endl;
	bdd::out(wcout, y) << endl;
	bdd::out(wcout, z) << endl << endl;
	wcout << allsat(x, 2) << endl;
	wcout << allsat(y, 2) << endl;
	wcout << allsat(z, 2) << endl;
	z = bdd::bdd_and(z, bdd::from_bit(2, false));
	bdd::out(wcout, z) << endl << endl;
	wcout << allsat(z, 3) << endl;
	z = bdd::bdd_or(x, y);
	wcout <<endl << allsat(z, 2) << endl;
	bdd::onexit = true;
	return 0;
}
