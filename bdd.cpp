#include <cassert>
#include <algorithm>
#include "bdd.h"
using namespace std;

int_t T, F;
unordered_map<bdd::key, int_t> bdd::M;
vector<bdd> bdd::V;
unordered_map<ite_memo, int_t> bdd::C;
unordered_map<bdds, int_t> bdd::AM;
unordered_set<int_t> bdd::S;
unordered_map<int_t, weak_ptr<bdd_handle>> bdd_handle::M;
typedef vector<bool> bools;
typedef vector<bools> vbools;

void bdd::init() {
	V.emplace_back(0, 0, 0), // dummy
	V.back().rehash(), T = 1, F = -1,
	V.emplace_back(0, 1, 1), V.back().rehash(),
	M.emplace(V.back().getkey(), 1), mark(0), mark(T), mark(F);
}

int_t bdd::add(uint_t v, int_t h, int_t l) {
	assert(v && h && l);
	if (h == l) return h;
	if (l < 0) {
		key k = { hash_tri(v, -h, -l), v, -h, -l };
		auto it = M.find(k);
		if (it != M.end()) return -it->second;
		V.emplace_back(v, -h, -l), M.emplace(k, V.size() - 1);
		return -V.size() + 1;
	}
	key k = { hash_tri(v, h, l), v, h, l };
	auto it = M.find(k);
	if (it != M.end()) return it->second;
	V.emplace_back(v, h, l), M.emplace(k, V.size() - 1);
	return V.size() - 1;
}

int_t bdd::from_bit(uint_t b, bool v) {
	return v ? add(b + 1, T, F) : add(b + 1, F, T);
}

int_t bdd::bdd_and(int_t x, int_t y) {
	assert(x && y);
	if (x == F || y == F) return F;
	if (x == T || x == y) return y;
	if (y == T) return x;
	if (x == -y) return F;
	if (x > y) swap(x, y);
	auto it = C.find({x,y,F});
	if (it != C.end()) return it->second;
	const uint_t vx = V[abs(x)].v, vy = V[abs(y)].v;
	int_t r;
	if (vx < vy) r = add(vx, bdd_and(hi(x), y), bdd_and(lo(x), y));
	else if (vx > vy) r = add(vy, bdd_and(x, hi(y)), bdd_and(x, lo(y)));
	else r = add(vx, bdd_and(hi(x), hi(y)), bdd_and(lo(x), lo(y)));
	return C.emplace(ite_memo{x,y,F}, r), r;
}

int_t bdd::bdd_ite(int_t x, int_t y, int_t z) {
	assert(x && y && z);
	if (x < 0) return bdd_ite(-x, z, y);
	if (x == F) return z;
	if (x == T || y == z) return y;
	if (x == -y || x == z) return F;
	auto it = C.find({x, y, z});
	if (it != C.end()) return it->second;
	int_t r;
	const uint_t vx = V[abs(x)].v, vy = V[abs(y)].v, vz = V[abs(z)].v;
	const uint_t s = min(vx, min(vy, vz));
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

size_t bdd::bdd_and_many_iter(bdds v, bdds& h, bdds& l, int_t &res, size_t &m) {
	size_t i;
	int_t t;
	bool b, eq, flag;
	bdds x;
	if (v.empty()) return res = T, 1;
	if (v.size() == 2) return res = bdd_and(v[0], v[1]), 1;
	for (size_t n = 0; n < v.size();)
		if (!leaf(v[n])) ++n;
		else if (!trueleaf(v[n])) return res = F, 1;
		else if (v.erase(v.begin()+n), v.size()==1) return res=v[0], 1;
		else if (v.size() == 2) return res = bdd_and(v[0], v[1]), 1;
		else if (v.empty()) return res = T, 1;
		else ++n;
	m = var(v[0]), t = v[0];
	b = false, eq = true, flag = false;
	for (i = 1; i != v.size(); ++i)
		if (!leaf(v[i])) {
			b |= var(v[i]) != m, eq &= t == v[i];
			if (var(v[i]) < m) m = var(v[i]);
		} else if (!trueleaf(v[i])) return res = F, 1;
	if (eq) return res = t, 1;
	for (i = 0; i != v.size(); ++i)
		if (b && var(v[i]) != m) h.push_back(v[i]);
		else if (!leaf(hi(v[i]))) h.push_back(hi(v[i]));
		else if (!trueleaf(hi(v[i]))) { flag = true; break; }
	for (i = 0; i != v.size(); ++i)
		if (b && var(v[i]) != m) l.push_back(v[i]);
		else if (!leaf(lo(v[i]))) l.push_back(lo(v[i]));
		else if (!trueleaf(lo(v[i]))) return flag ? res = F, 1 : 2;
	auto f = [](int_t x, int_t y) { return abs(x) < abs(y); };
	sort(h.begin(), h.end(), f), sort(l.begin(), l.end(), f);
	if (!flag) {
		for (size_t n = 1; n < h.size();)
			if (h[n] == -h[n-1]) { flag = true; break; }
			else if (h[n] == h[n-1]) {
				h.erase(h.begin() + n);
				if (h.empty()) { flag = true; break; }
				if (h.size() == 1) break;
			} else ++n;
	}
	for (size_t n = 1; n < l.size();)
		if (l[n] == -l[n-1]) return flag ? 3 : 0;
		else if (l[n] == l[n-1]) {
			l.erase(l.begin() + n);
			if (l.empty()) return flag ? 3 : 0;
			if (l.size() == 1) break;
		} else ++n;
	if (flag) return 3;
	set_intersection(h.begin(),h.end(),l.begin(),l.end(),back_inserter(x));
	if (x.size() > 1) {
		for (size_t n = 0; n < h.size();)
			if (hasb(x, h[n])) h.erase(h.begin() + n);
			else ++n;
		for (size_t n = 0; n < l.size();)
			if (hasb(x, l[n])) l.erase(l.begin() + n);
			else ++n;
		int_t r = bdd_and_many(move(x));
		if (r == F) return res = F, 1;
		h.push_back(r), l.push_back(r);
	}
	return 0;
}

int_t bdd::bdd_and_many(bdds v) {
	static unordered_map<ite_memo, int_t>::const_iterator jt;
	for (size_t n = 0; n < v.size(); ++n)
		for (size_t k = 0; k < n; ++k) {
			int_t x, y;
			if (v[n] < v[k]) x = v[n], y = v[k];
			else x = v[k], y = v[n];
			if ((jt = C.find({x, y, F})) != C.end()) {
				v.erase(v.begin()+k), v.erase(v.begin()+n-1),
				v.push_back(jt->second), n = k = 0;
				break;
			}
		}
	if (v.empty()) return T;
	if (v.size() == 1) return v[0];
	if (v.size() == 2) return bdd_and(v[0], v[1]);
	auto it = AM.find(v);
	if (it != AM.end()) return it->second;
//	onmemo();
	int_t res = F, h, l;
	size_t m = 0;
	bdds vh, vl;
	switch (bdd_and_many_iter(move(v), vh, vl, res, m)) {
		case 0: l = bdd_and_many(move(vl)),
			h = bdd_and_many(move(vh));
			break;
		case 1: return AM.emplace(v, res), res = res;
		case 2: h = bdd_and_many(move(vh)), l = F; break;
		case 3: h = F, l = bdd_and_many(move(vl)); break;
		default: throw 0;
	}
	return AM.emplace(v, bdd::add(m, h, l)).first->second;
}

void bdd::gc() {
	if (V.size() / S.size() <= 2) return;
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
	for (size_t n = 2; n != V.size(); ++n)
		f(V[n].h), f(V[n].l), V[n].rehash();
	unordered_map<ite_memo, int_t> c;
	unordered_map<bdds, int_t> am;
	for (pair<ite_memo, int_t> x : C)
		if (!(	has(G, abs(x.first[0])) ||
			has(G, abs(x.first[1])) ||
			has(G, abs(x.first[2])) ||
			has(G, abs(x.second))))
			f(x.first[0]), f(x.first[1]), f(x.first[2]),
			f(x.second), c.emplace(x.first, x.second);
	bool b;
	for (pair<bdds, int_t> x : AM) {
		b = false;
		for (int_t& i : x.first)
			if (b |= has(G, abs(i))) break;
			else f(i);
		if (b || has(G, x.second)) continue;
		f(x.second), am.emplace(x.first, x.second);
	}
	wcout << c.size() << ' ' << C.size();
	bdd_handle::update(p, G);
	C = move(c), AM = move(am), G.clear();
	for (size_t n = 0; n < V.size(); ++n) M.emplace(V[n].getkey(), n);
	wcout << ' ' << V.size() << endl;
}

void bdd_handle::update(const vector<int_t>& p, const unordered_set<int_t>& G) {
	std::unordered_map<int_t, std::weak_ptr<bdd_handle>> m;
	for (pair<int_t, std::weak_ptr<bdd_handle>> x : m)
		assert(!has(G, x.first)), f(x.first), f(x.second.lock()->b),
		m.emplace(x.first,x.second);
	M = move(m);
}
#undef f

void sat(uint_t v, uint_t nvars, int_t t, bools& p, vbools& r) {
	if (bdd::leaf(t) && !bdd::trueleaf(t)) return;
	if (!bdd::leaf(t) && v < getnode(abs(t)).v)
		p[v - 1] = true, sat(v + 1, nvars, t, p, r),
		p[v - 1] = false, sat(v + 1, nvars, t, p, r);
	else if (v != nvars) {
		p[v - 1] = true, sat(v + 1, nvars, bdd::hi(t), p, r),
		p[v - 1] = false, sat(v + 1, nvars, bdd::lo(t), p, r);
	} else	r.push_back(p);
}

vbools allsat(int_t x, uint_t nvars) {
	bools p(nvars);
	vbools r;
	return sat(1, nvars + 1, x, p, r), r;
}

size_t std::hash<ite_memo>::operator()(const ite_memo& m) const {
	return hash_pair(m[0], hash_pair(m[1], m[2]));
}

size_t std::hash<bdds>::operator()(const bdds& b) const {
	size_t r = 0;
	for (int_t i : b) r += i >= 0 ? bdd::V[i].hash : -bdd::V[-i].hash;
	return r;
}

bdd::bdd(uint_t v, int_t h, int_t l) : h(h), l(l), v(v) { rehash(); }

wostream& bdd::out(wostream& os, int_t x) {
	if (leaf(x)) return os << (trueleaf(x) ? L'T' : L'F');
	if (x < 0) x = -x, os << L'~';
	const bdd& b = getnode(x);
	return out(out(os << b.v << L" ? ", b.h) << L" : ", b.l);
}

wostream& operator<<(wostream& os, const bools& x) {
	for (auto y : x) os << (y ? 1 : 0);
	return os;
}

wostream& operator<<(wostream& os, const vbools& x) {
	for (auto y : x) os << y << endl;
	return os;
}

int_t rand_bdd(int_t n = 5) {
	if (!n) return bdd::bdd_ite(
			bdd::from_bit(random()%10, random()&1),
			bdd::from_bit(random()%10, random()&1),
			bdd::from_bit(random()%10, random()&1));
	return bdd::bdd_ite(rand_bdd(n-1), rand_bdd(n-1), rand_bdd(n-1));
}

void test_and_many() {
	set<spbdd_handle> s;
	for (size_t k = 0; k != 100; ++k) {
		bdds b;
		for (size_t n = 0; n != 8; ++n) b.push_back(rand_bdd());
		int_t r = T;
		for (int_t i : b) r = bdd::bdd_and(r, i);
		assert(r == bdd::bdd_and_many(b));
		cout<<k<<endl;
		if (random()&1) s.insert(bdd_handle::get(r));
		bdd::gc();
	}
}

int main() {
	bdd::init();
	test_and_many();
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
//	z = bdd::bdd_and(z, bdd::from_bit(2, false));
//	bdd::out(wcout, z) << endl << endl;
//	wcout << allsat(z, 3) << endl;
//	z = bdd::bdd_or(x, y);
//	wcout <<endl << allsat(z, 2) << endl;
	return 0;
}
