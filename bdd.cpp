#include <iostream>
#include "bdd.h"
using namespace std;

int_t T, F;
unordered_map<bdd, int_t> bdd::M;
vector<bdd> bdd::V;
unordered_map<ite_memo, int_t> bdd::C;
set<int_t> bdd::G;
bool bdd::onexit = false;

int_t bdd_and(int_t x, int_t y) {
	if (x == F || y == F) return F;
	if (x == T || x == y) return y;
	if (y == T) return x;
	if (x == -y) return F;
	const bdd &a = getnode(abs(x)), &b = getnode(abs(y));
	if (a.v < b.v)
		return bdd::add(a.v, bdd_and(a.h, y), bdd_and(a.l, y));
	if (a.v > b.v)
		return bdd::add(b.v, bdd_and(x, b.h), bdd_and(x, b.l));
	return bdd::add(a.v, bdd_and(a.h, b.h), bdd_and(a.l, b.l));
}

void sat(size_t v, size_t nvars, int_t t, std::vector<bool>& p, std::vector<std::vector<bool>>& r) {
	if (bdd::leaf(t) && !bdd::trueleaf(t)) return;
	const bdd &x = getnode(abs(t));
	if (!bdd::leaf(t) && v < x.v)
		p[v-1] = true,  sat(v + 1, nvars, t, p, r),
		p[v-1] = false, sat(v + 1, nvars, t, p, r);
	else if (v != nvars)
		p[v-1] = true,  sat(v + 1, nvars, x.h, p, r),
		p[v-1] = false, sat(v + 1, nvars, x.l, p, r);
	else	r.push_back(p);
}

std::vector<std::vector<bool>> allsat(int_t x, size_t nvars) {
	std::vector<bool> p(nvars);
	std::vector<std::vector<bool>> r;
	return sat(1, nvars + 1, x, p, r), r;
}

size_t std::hash<bdd>::operator()(const bdd& b) const { return b.hash; }

size_t std::hash<ite_memo>::operator()(const ite_memo& m) const {
	return hash_pair(m[0].hash, hash_pair(m[1].hash, m[2].hash));
}

bdd::bdd(size_t v, int_t h, int_t l) : v(v), refs(1), h(h), l(l) {
	rehash(), ++getnode(abs(h)).refs, ++getnode(abs(l)).refs;
}

bdd::~bdd() {
	if (onexit) return;
	decref(), getnode(abs(h)).decref(), getnode(abs(l)).decref();
}

void bdd::decref() { if (refs && !--refs) G.insert(this - &V[0]); }

wostream& operator<<(wostream& os, const std::vector<bool>& x) {
	for (auto y : x) os << (y ? 1 : 0);
	return os;
}

wostream& operator<<(wostream& os, const std::vector<std::vector<bool>>& x) {
	for (auto y : x) os << y << endl;
	return os;
}

int main() {
	bdd::init();
	int_t x = bdd::from_bit(0, true);
	int_t y = bdd::from_bit(1, false);
	int_t z = bdd_and(x, y);
	wcout << allsat(x, 2) << endl << endl;
	wcout << allsat(y, 2) << endl << endl;
	wcout << allsat(z, 3) << endl;
	bdd::onexit = true;
	return 0;
}
