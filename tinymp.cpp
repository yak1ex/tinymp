#include <vector>
#include <iostream>
#include <sstream>
#include <numeric>
#include <algorithm>
#include <utility>
#include <cassert>
#include <iterator>

class tinymp
{
	typedef unsigned int elem_type;
	typedef unsigned long long widen_type;
	std::vector<elem_type> v;
	bool nonneg;
	void normalize() {
		while(v.size() > 1 && v.back() == 0) v.pop_back();
		if(is_zero()) nonneg = true;
	}
	class offseter
	{
		const std::vector<elem_type> &pv;
		static elem_type zero;
		std::size_t offset;
	public:
		offseter(const std::vector<elem_type>& pv_, std::size_t offset_):pv(pv_),offset(offset_) {}
		class iterator {
			const offseter *p;
			std::size_t pos;
		public:
			typedef std::random_access_iterator_tag iterator_category;
			typedef elem_type value_type;
			typedef std::ptrdiff_t difference_type;
			typedef const elem_type* pointer;
			typedef const elem_type& reference;
			iterator(const offseter *p_, std::size_t pos_) : p(p_), pos(pos_) {}
			reference operator*() { return pos >= p->offset ? p->pv[pos - p->offset] : zero; }
			iterator& operator++() { ++pos; return *this; }
			iterator operator++(int) { iterator ret(*this); ++pos; return std::move(ret); }
			iterator& operator--() { --pos; return *this; }
			iterator operator--(int) { iterator ret(*this); --pos; return std::move(ret); }
			bool operator==(const iterator& other) const { return p == other.p && pos == other.pos; }
			bool operator!=(const iterator& other) const { return !(*this == other); }
			iterator operator-=(difference_type n) { pos -= n; return *this; }
			iterator operator+=(difference_type n) { pos += n; return *this; }
			iterator operator-(difference_type n) const { iterator r(*this); return std::move(r -= n); }
			iterator operator+(difference_type n) const { iterator r(*this); return std::move(r += n); }
			difference_type operator-(const iterator& other) const { return pos - other.pos; }
		};
		typedef std::reverse_iterator<iterator> reverse_iterator;
		const elem_type& operator[](std::size_t idx) { return idx >= offset ? pv[idx - offset] : zero; }
		iterator begin() { return iterator(this, 0); }
		iterator end() { return iterator(this, pv.size() + offset); }
		reverse_iterator rbegin() { return reverse_iterator(end()); }
		reverse_iterator rend() { return reverse_iterator(begin()); }
		std::size_t size() const { return pv.size() + offset; }
	};
	void addsub(const std::vector<elem_type> &ov, bool oneg, std::size_t offset = 0) {
		if(nonneg == oneg) {
			offseter ov_(ov, offset);
			if(ov_.size() > v.size()) v.resize(ov_.size());
			elem_type carry = 0;
			for(std::size_t i = 0; i < v.size(); ++i) {
				elem_type new_carry;
				elem_type oval = i < ov_.size() ? ov_[i] : 0;
				if(std::numeric_limits<elem_type>::max() - oval < v[i] + carry) {
					new_carry = 1;
				} else {
					new_carry = 0;
				}
				v[i] += oval + carry;
				carry = new_carry;
			}
			if(carry) v.push_back(1);
		} else {
			bool sub = absless(ov, offset);
			offseter ov_(ov, offset);
			offseter v_(v, 0);
			if(ov_.size() > v.size()) v.resize(ov_.size());
			offseter &lhs = sub ? ov_ : v_;
			offseter &rhs = sub ? v_ : ov_;
			// ensures v.size() == lhs.size() >= rhs.size()
			elem_type borrow = 0;
			for(std::size_t i = 0; i < v.size(); ++i) {
				elem_type new_borrow;
				elem_type rval = i < rhs.size() ? rhs[i] : 0;
				if(lhs[i] < rval + borrow) {
					new_borrow = 1;
				} else {
					new_borrow = 0;
				}
				v[i] = lhs[i] - rval - borrow;
				borrow = new_borrow;
			}
			assert(borrow == 0);
			normalize();
			nonneg = sub ? oneg : nonneg;
		}
	}
	bool absless(const std::vector<elem_type> &ov, std::size_t offset = 0) const {
		offseter ov_(ov, offset);
		if(v.size() < ov_.size()) return true;
		if(v.size() > ov_.size()) return false;
		return std::lexicographical_compare(v.rbegin(), v.rend(), ov_.rbegin(), ov_.rend());
	}
	bool absgreater(const std::vector<elem_type> &ov) const {
		if(v.size() < ov.size()) return false;
		if(v.size() > ov.size()) return true;
		return std::lexicographical_compare(ov.rbegin(), ov.rend(), v.rbegin(), v.rend());
	}
	bool absequal(const std::vector<elem_type> &ov) const {
		if(v.size() != ov.size()) return false;
		return std::equal(v.begin(), v.end(), ov.begin());
	}
	bool is_zero() const {
		return v.size() == 1 && v[0] == 0;
	}
	tinymp& from_chars(const char* p, std::size_t size) {
		if(size == 0) return *this; // might be better to throw
		auto it = p, it_end = p + size;
		*this = 0;
		bool negative = false;
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
	void dump(std::ostream &os) const {
		for(auto val : v) {
			os << val << ',';
		}
		os << (nonneg ? '+' : '-') << std::endl;
	}
public:
	tinymp(elem_type val = 0): v(1, val),nonneg(true) {}
	// compound assignments
	tinymp& operator+=(const tinymp& other) {
		addsub(other.v, other.nonneg);
		return *this;
	}
	tinymp& operator-=(const tinymp& other) {
		if(!other.is_zero()) addsub(other.v, !other.nonneg);
		return *this;
	}
	tinymp& operator*=(elem_type s) {
		widen_type carry = 0;
		for(std::size_t i = 0; i < v.size(); ++i) {
			widen_type temp = widen_type(v[i]) * s + carry;
			v[i] = temp;
			carry = temp >> std::numeric_limits<elem_type>::digits;
		}
		if(carry != 0) v.push_back(carry);
		return *this;
	}
	tinymp& operator*=(const tinymp& other) {
		*this = std::move(*this * other);
		return *this;
	}
	tinymp& operator/=(elem_type s) {
		return div_(s).first;
	}
	tinymp& operator/=(const tinymp& other) {
		return div_(other).first;
	}
	tinymp& operator%=(elem_type s) {
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
		return std::move(r.flip_());
	}
	// arithmetic binary operator helper
	std::pair<tinymp&, elem_type> div_(elem_type s) {
		widen_type unit = widen_type(1) << std::numeric_limits<elem_type>::digits;
		widen_type borrow = 0;
		for(std::size_t i = 0; i < v.size(); ++i) {
			auto temp = borrow * unit + v[v.size() - i - 1];
			v[v.size() - i - 1] = temp / s;
			borrow = temp % s;
		}
		normalize();
		return { *this, borrow };
	}
	std::pair<tinymp, elem_type> div(elem_type s) const {
		return tinymp(*this).div_(s);
	}
	std::pair<tinymp&, tinymp> div_(const tinymp& other) {
		auto t = tinymp(*this).div(other);
		*this = std::move(t.first);
		return { *this, std::move(t.second) };
	}
	std::pair<tinymp, tinymp> div(const tinymp& other) const {
		if(!(absless(other))) {
			tinymp r;
			r.v.resize(v.size() - other.v.size() + 1);
			tinymp residual(*this);
			for(std::size_t i = 0; i < r.v.size(); ++i) {
				std::size_t idxr = r.v.size() - i - 1;
				if(residual.v.size() < idxr + other.v.size() || residual.absless(other.v, idxr)) continue;
				widen_type res = 0;
				if(residual.v.size() > idxr + other.v.size()) {
					res = widen_type(residual.v[idxr + other.v.size()]) << std::numeric_limits<elem_type>::digits;
				}
				res += residual.v[idxr + other.v.size() - 1];
				widen_type top = other.v.back();
				if(0 <= idxr + other.v.size() - 2 && other.v.size() - 2 >= 0) {
					std::size_t bres = std::numeric_limits<widen_type>::digits - 1;
					while(!((widen_type(1) << bres) & res) && bres > 0) --bres;
					std::size_t btop = std::numeric_limits<widen_type>::digits - 1;
					while(!((widen_type(1) << btop) & top) && btop > 0) --btop;
					std::size_t bits = std::min<widen_type>(32, std::numeric_limits<widen_type>::digits - std::max(bres, btop) - 1);
					res <<= bits;
					res += residual.v[idxr + other.v.size() - 2] >> (std::numeric_limits<elem_type>::digits - bits);
					top <<= bits;
					top += other.v[other.v.size() - 2] >> (std::numeric_limits<elem_type>::digits - bits);
				}
				elem_type candidate = res / top;
				if(candidate > 0) --candidate;
				auto temp = other * candidate;
				while(!residual.absless(temp.v, idxr)) {
					++candidate;
					temp = other * candidate;
				}
				--candidate;
				temp = other * candidate;
				residual.addsub(temp.v, false, idxr); // residual -= (temp << (idxr * BITS));
				r.v[idxr] = candidate;
			}
			r.normalize();
			r.nonneg = !(nonneg ^ other.nonneg);
			return { std::move(r), residual };
		} else {
			return { {}, *this };
		}
	}
	// arithmetic binary operators
	friend inline tinymp operator+(tinymp v1, const tinymp &v2) {
		return std::move(v1 += v2);
	}
	friend inline tinymp operator-(tinymp v1, const tinymp &v2) {
		return std::move(v1 -= v2);
	}
	friend inline tinymp operator*(tinymp other, elem_type s) {
		return std::move(other *= s);
	}
	// TODO: Karatsuba algorithm?
	friend tinymp operator*(const tinymp &v1, const tinymp& v2) {
		tinymp r;
		std::vector<elem_type> tempv(2);
		r.v.resize(v1.v.size() + v2.v.size());
		for(std::size_t i = 0; i < v1.v.size(); ++i) {
			for(std::size_t j = 0; j < v2.v.size(); ++j) {
				widen_type temp = widen_type(v1.v[i]) * v2.v[j];
				tempv[0] = temp;
				tempv[1] = temp >> std::numeric_limits<elem_type>::digits;
				r.addsub(tempv, true, i+j);
			}
		}
		r.normalize();
		r.nonneg = !(v1.nonneg ^ v2.nonneg);
		return std::move(r);
	}
	friend inline tinymp operator/(tinymp other, elem_type s) {
		return std::move(other /= s);
	}
	friend inline tinymp operator/(tinymp v1, const tinymp &v2) {
		return std::move(v1 /= v2);
	}
	friend inline elem_type operator%(tinymp other, elem_type s) {
		return other.div_(s).second;
	}
	friend inline tinymp operator%(tinymp v1, const tinymp &v2) {
		return std::move(v1.div_(v2).second);
	}
	// TODO: increment/decrement
	// TODO: shift
	// TODO: bit-wise arithmetic
	// comparison
	bool absless(const tinymp &other) const {
		return absless(other.v);
	}
	bool absgreater(const tinymp &other) const {
		return absgreater(other.v);
	}
	bool absequal(const tinymp &other) const {
		return absequal(other.v);
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
	friend inline tinymp operator"" _tmp();
	// I/O
	// FIXME: exception safety
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
			while(!v.is_zero()) { auto t = v.div_(10); s.push_back(t.second + '0'); }
			std::reverse(s.begin() + (negative ? 1 : 0), s.end());
		}
		return std::move(s);
	}
	friend inline std::ostream& operator<<(std::ostream &os, tinymp v) {
		return os << to_string(v);
	}
};
tinymp::elem_type tinymp::offseter::zero = 0;
template<char ... c>
inline tinymp operator"" _tmp()
{
	std::string s({c...});
	return std::move(tinymp().from_chars(s.data(), s.size()));
}
