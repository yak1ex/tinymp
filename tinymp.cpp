#include <vector>
#include <numeric>
#include <algorithm>
#include <utility>
#include <tuple>
#include <limits>
#include <iostream>

// tiny multi-precision integer class
// distributed under the CC0-1.0
class tinymp
{
private:
	typedef unsigned int value_type;
	typedef std::numeric_limits<value_type> limits_type;
	typedef unsigned long long widen_type;
	typedef std::numeric_limits<widen_type> wlimits_type;
	typedef std::vector<value_type> vector_type;
	typedef const std::vector<value_type> cvector_type;

public:
	tinymp(value_type val = 0, bool nonneg_ = true): v(1, val), nonneg(val == 0 || nonneg_ ) {}
	tinymp(const char* p, std::size_t sz) { from_chars(p, sz); }
	template<typename InIt>
	tinymp(InIt it1, InIt it2): v(it1, it2), nonneg(true) {
		if(v.size() == 0) v.push_back(0);
	}
	// compound assignments
	tinymp& operator+=(const tinymp& other) {
		if(nonneg ^ other.nonneg) {
			nonneg ^= sub(v, other.v);
		} else {
			add(v, other.v);
		}
		return *this;
	}
	tinymp& operator-=(const tinymp& other) {
		if(!other.is_zero()) {
			if(nonneg ^ other.nonneg) {
				add(v, other.v);
			} else {
				nonneg ^= sub(v, other.v);
			}
		}
		return *this;
	}
	tinymp& operator*=(value_type s) {
		widen_type carry = 0;
		switch(s) {
		case 0:
			*this = 0;
			break;
		case 1:
			break; // do nothing
		default:
			for(std::size_t i = 0; i < v.size(); ++i) {
				widen_type temp = widen_type(v[i]) * s + carry;
				v[i] = temp;
				carry = temp >> limits_type::digits;
			}
			if(carry != 0) v.push_back(carry);
		}
		return *this;
	}
	tinymp& operator*=(const tinymp& other) {
		*this = std::move(*this * other);
		return *this;
	}
	tinymp& operator/=(value_type s) {
		return div_(s).first;
	}
	tinymp& operator/=(const tinymp& other) {
		return div_(other).first;
	}
	tinymp& operator%=(value_type s) {
		*this = div_(s).second;
		return *this;
	}
	tinymp& operator%=(const tinymp& other) {
		*this = div_(other).second;
		return *this;
	}
	// arithmetic unary operators
	tinymp operator+() const {
		return tinymp(*this);
	}
	tinymp& flip_() {
		if(v.size() != 1 || v[0] != 0) nonneg = !nonneg;
		return *this;
	}
	tinymp operator-() const {
		tinymp r(*this);
		r.flip_();
		return r; // NRVO
	}
	// arithmetic binary operator helper
	std::pair<tinymp&, tinymp> div_(value_type s) {
		widen_type unit = widen_type(1) << limits_type::digits;
		widen_type borrow = 0;
		for(std::size_t i = 0; i < v.size(); ++i) {
			auto temp = borrow * unit + v[v.size() - i - 1];
			v[v.size() - i - 1] = temp / s;
			borrow = temp % s;
		}
		normalize();
		return { std::piecewise_construct, std::forward_as_tuple(*this), std::forward_as_tuple(borrow, nonneg) };
	}
	std::pair<tinymp, tinymp> div(value_type s) const {
		return tinymp(*this).div_(s);
	}
	std::pair<tinymp&, tinymp> div_(const tinymp& other) {
		auto t = tinymp(*this).div(other);
		*this = std::move(t.first);
		return { std::piecewise_construct, std::forward_as_tuple(*this), std::forward_as_tuple(std::move(t.second)) };
	}
	std::pair<tinymp, tinymp> div(const tinymp& other) const {
		// TODO: check Knuth algorithm
		std::pair<tinymp, tinymp> p{ std::piecewise_construct, std::forward_as_tuple(), std::forward_as_tuple(*this) };
		if(!(absless(other))) {
			tinymp& r = p.first;
			r.v.resize(v.size() - other.v.size() + 1);
			tinymp & residual = p.second;
			residual.nonneg = true;
			for(std::size_t i = 0; i < r.v.size(); ++i) {
				std::size_t idxr = r.v.size() - i - 1;
				if(residual.v.size() < idxr + other.v.size() || absless(residual.v, coffseter_type(other.v, idxr))) continue;
				widen_type res = 0;
				if(residual.v.size() > idxr + other.v.size()) {
					res = widen_type(residual.v[idxr + other.v.size()]) << limits_type::digits;
				}
				res += residual.v[idxr + other.v.size() - 1];
				widen_type top = other.v.back();
				if(2 <= idxr + other.v.size() && other.v.size() >= 2) {
					auto bres = wlimits_type::digits;
					while(bres > 0 && !((widen_type(1) << (bres-1)) & res)) --bres;
					auto btop = wlimits_type::digits;
					while(btop > 0 && !((widen_type(1) << (btop-1)) & top)) --btop;
					auto bits = std::min<std::size_t>(32, wlimits_type::digits - std::max(bres, btop));
					if(bits) {
						res <<= bits;
						res += residual.v[idxr + other.v.size() - 2] >> (limits_type::digits - bits);
						top <<= bits;
						top += other.v[other.v.size() - 2] >> (limits_type::digits - bits);
					}
				}
				value_type candidate = res / top;
				if(candidate > 0) --candidate;
				auto temp = other * candidate;
				while(candidate == 0 || !absless(residual.v, offseter_type(temp.v, idxr))) {
					++candidate;
					temp += other; // temp = other * candidate seems to be faster if other has 1000 digits (base-10) order
				}
				--candidate;
				temp -= other; // temp = other * candidate seems to be faster if other has 1000 digits (base-10) order
				sub(residual.v, offseter_type(temp.v, idxr)); // residual -= (temp << (idxr * BITS));
				r.v[idxr] = candidate;
			}
			r.normalize();
			r.nonneg = !(nonneg ^ other.nonneg);
			if(!nonneg) residual.flip_();
		}
		return p;
	}
	// arithmetic binary operators
	friend inline tinymp operator+(const tinymp &v1, const tinymp &v2) {
		tinymp r(v1); r += v2; return r;
	}
	friend inline tinymp operator-(tinymp v1, const tinymp &v2) {
		return std::move(v1 -= v2);
	}
	friend inline tinymp operator*(tinymp other, value_type s) {
		return std::move(other *= s);
	}
	friend tinymp operator*(const tinymp &v1, const tinymp& v2) {
		tinymp r;
		vector_type tempv(2);
		r.v.resize(v1.v.size() + v2.v.size());
		for(std::size_t i = 0; i < v1.v.size(); ++i) {
			for(std::size_t j = 0; j < v2.v.size(); ++j) {
				widen_type temp = widen_type(v1.v[i]) * v2.v[j];
				tempv[0] = temp;
				tempv[1] = temp >> limits_type::digits;
				add(r.v, offseter_type(tempv, i+j));
			}
		}
		r.normalize();
		r.nonneg = !(v1.nonneg ^ v2.nonneg);
		return r; // NRVO
	}
	friend inline tinymp operator/(const tinymp &other, value_type s) {
		tinymp r(other); r /= s; return r; // NRVO
	}
	friend inline tinymp operator/(const tinymp &v1, const tinymp &v2) {
		tinymp r(v1); r /= v2; return r; // NRVO
	}
	friend inline tinymp operator%(const tinymp &other, value_type s) {
		return tinymp(other).div_(s).second;
	}
	friend inline tinymp operator%(const tinymp &v1, const tinymp &v2) {
		return tinymp(v1).div_(v2).second;
	}
	// TODO: increment/decrement
	// TODO: shift
	// TODO: bit-wise arithmetic
	// comparison
	bool absless(const tinymp &other) const {
		return absless(v, other.v);
	}
	bool absgreater(const tinymp &other) const {
		return absgreater(v, other.v);
	}
	bool absequal(const tinymp &other) const {
		if(v.size() != other.v.size()) return false;
		return std::equal(v.begin(), v.end(), other.v.begin());
	}
	friend inline bool operator==(const tinymp &v1, const tinymp &v2) {
		if(v1.nonneg != v2.nonneg) return false;
		return v1.absequal(v2);
	}
	friend inline bool operator!=(const tinymp &v1, const tinymp &v2) {
		return !(v1 == v2);
	}
	friend inline bool operator<(const tinymp &v1, const tinymp &v2) {
		if(!v1.nonneg && v2.nonneg) return true;
		if(v1.nonneg && !v2.nonneg) return false;
		if(v1.nonneg) return v1.absless(v2);
		return v1.absgreater(v2);
	}
	friend inline bool operator>(const tinymp &v1, const tinymp &v2) {
		return v2 < v1;
	}
	friend inline bool operator<=(const tinymp &v1, const tinymp &v2) {
		return !(v2 < v1);
	}
	friend inline bool operator>=(const tinymp &v1, const tinymp &v2) {
		return !(v2 > v1);
	}
	// literal
	template<char ... c>
	static inline tinymp literal()
	{
		std::string s({c...});
		return tinymp(s.data(), s.size()); // RVO
	}
	// I/O
	static inline tinymp stotmp(const std::string &s) {
		return tinymp(s.data(), s.size()); // RVO
	}
	friend inline std::istream& operator>>(std::istream &is, tinymp& v) {
		std::string s;
		is >> s;
		v.from_chars(s.data(), s.size());
		return is;
	}
	friend inline std::string to_string(tinymp v) {
		std::string s;
		bool negative = !v.nonneg;
		if(v.is_zero()) s = "0";
		else {
			if(negative) s = "-";
			while(!v.is_zero()) { auto t = v.div_(10); s.push_back(t.second.v[0] + '0'); }
			std::reverse(s.begin() + (negative ? 1 : 0), s.end());
		}
		return s; // NRVO
	}
	friend inline std::ostream& operator<<(std::ostream &os, const tinymp& v) {
		return os << to_string(v);
	}

private:
	vector_type v;
	bool nonneg;
	void normalize() {
		while(v.size() > 1 && v.back() == 0) v.pop_back();
		if(is_zero()) nonneg = true;
	}

	template<typename T> // T = vector<...>
	class offseter // minimal offset vector access
	{
	private:
		typedef decltype(std::declval<T>().begin()) iterator_type;
		class riterator { // lexicographical_compare() requires InputIterator
			const offseter *p;
			std::size_t pos; // [0, p->sz + p->offset]: (p->sz + p->off) points to p->st[p->sz - 1]
		public:
			typedef std::input_iterator_tag iterator_category;
			typedef value_type value_type;
			typedef std::ptrdiff_t difference_type;
			typedef const value_type* pointer;
			typedef const value_type& reference;
			riterator(const offseter *p_, std::size_t pos_) : p(p_), pos(pos_) {}
			reference operator*() { return pos >= p->off + 1 ? p->st[pos - p->off - 1] : zero(); }
			riterator& operator++() { --pos; return *this; }
			riterator operator++(int) { riterator r(*this); --pos; return r; /* NRVO */ }
			bool operator==(const riterator& other) const { return p == other.p && pos == other.pos; }
			bool operator!=(const riterator& other) const { return !(*this == other); }
		};
	public:
		typedef typename T::value_type value_type;
		offseter(T& pv_, std::size_t offset_ = 0) : st(pv_.begin()), sz(pv_.size()), cap(pv_.capacity()), off(offset_) {}
		template<typename U> // should limit U
		offseter(const offseter<U>& o) : st(o.start()), sz(o.size() - o.offset()), cap(o.capacity()), off(o.offset()) {}
		const value_type& operator[](std::size_t idx) const { return idx >= off ? st[idx - off] : zero(); }
		value_type& operator[](std::size_t idx) { return idx >= off ? st[idx - off] : zero(); }
		riterator rbegin() const { return riterator(this, sz + off); }
		riterator rend() const { return riterator(this, 0); }
		std::size_t size() const { return sz + off; }
		std::size_t offset() const { return off; }
		std::size_t capacity() const { return cap; }
		iterator_type start() const { return st; }
		void resize(std::size_t sz_) { if(sz_ <= cap + off && sz_ > off) { for(std::size_t idx = sz; idx < sz_ - off; ++idx) st[idx] = 0; sz = sz_ - off; } else throw std::bad_alloc(); }
		void push_back(const value_type& t) { if(sz < cap) { st[sz] = t; ++sz; } else throw std::bad_alloc(); }
	private:
		iterator_type st;
		std::size_t sz; // valid digits
		std::size_t cap;
		std::size_t off;
		static value_type& zero() {
			static value_type zero_;
			return zero_;
		}
	};
	typedef offseter<vector_type> offseter_type;
	typedef offseter<cvector_type> coffseter_type;
	template<typename T>
	static bool overflow(T carry, T v1, T v2) {
		return carry ? v1 <= v2 : v1 < v2;
	}
	// absolute value arith
	template<typename T, typename U> // T, U are vector<V> or offseter<V>
	static bool absless(const T &v1, const U &v2) {
		if(v1.size() < v2.size()) return true;
		if(v1.size() > v2.size()) return false;
		return std::lexicographical_compare(v1.rbegin(), v1.rend(), v2.rbegin(), v2.rend());
	}
	template<typename T, typename U> // T, U are vector<V> or offseter<V>
	static bool absgreater(const T &v1, const U &v2) {
		if(v1.size() < v2.size()) return false;
		if(v1.size() > v2.size()) return true;
		return std::lexicographical_compare(v2.rbegin(), v2.rend(), v1.rbegin(), v1.rend());
	}
	template<typename T>
	static std::size_t offset(const std::vector<T>&) { return 0; }
	template<typename T> // T = vector<...>
	static std::size_t offset(const offseter<T>& t) { return t.offset(); }
	template<typename T, typename U> // T, U are vector<V> or offseter<V>
	static void add(T &v1, const U &v2) {
		if(v1.size() < v2.size()) v1.resize(v2.size());
		value_type carry = 0;
		for(std::size_t idx = offset(v2); idx < v1.size(); ++idx) {
			value_type val = idx < v2.size() ? v2[idx] : 0;
			value_type new_carry = overflow(carry,  limits_type::max() - val,  v1[idx]);
			v1[idx] += val + carry;
			carry = new_carry;
			if(idx >= v2.size() && carry  == 0) break;
		}
		if(carry) v1.push_back(1);
	}
	template<typename T> // T is vector<V> or offseter<V>
	static void normalize(T &v) {
		auto idx = v.size() - 1; // size() should be more than 0
		for(;v[idx] == 0 && idx > 0; --idx);
		v.resize(idx + 1);
	}
	template<typename T, typename U> // T, U are vector<V> or offseter<V>
	static bool sub(T &v1, const U &v2) {
		bool sub = absless(v1, v2);
		if(v1.size() < v2.size()) v1.resize(v2.size());
		const coffseter_type &lhs = sub ? v2 : v1;
		const coffseter_type &rhs = sub ? v1 : v2;
		if(lhs.offset() > rhs.offset()) throw std::invalid_argument("offset mismatch");
		// NOTE: v1.size() == lhs.size() >= rhs.size() && lhs >= rhs
		value_type borrow = 0;
		std::size_t idx;
		for(idx = rhs.offset(); idx < lhs.size(); ++idx) {
			value_type rval = idx < rhs.size() ? rhs[idx] : 0;
			// lhs[idx] < rval + borrow
			value_type new_borrow = overflow(borrow, lhs[idx], rval);
			v1[idx] = lhs[idx] - rval - borrow;
			borrow = new_borrow;
			if( idx >= rhs.size() && borrow == 0) break;
		}
	 	if(borrow != 0) throw std::logic_error("Can't be borrow != 0");
		normalize(v1);
		return sub;
	}

	bool is_zero() const {
		return v.size() == 1 && v[0] == 0;
	}
	tinymp& from_chars(const char* p, std::size_t size) {
		if(size == 0) return *this; // might be better to throw
		auto it = p, it_end = p + size;
		*this = 0;
		auto negative = false;
		if(*it == '-') {
			negative = true;
			++it;
		}
		while(it != it_end) {
			if('0' <= *it && *it <= '9') {
				*this *= 10;
				*this += (*it - '0');
			} // otherwise, might be better to throw
			++it;
		}
		if(negative) flip_();
		return *this;
	}
};
template<char ... c>
inline tinymp operator"" _tmp()
{
	return tinymp::literal<c...>(); // RVO
}
inline tinymp stotmp(const std::string& s)
{
	return tinymp::stotmp(s); // RVO
}
