#include <vector>
#include <iostream>
#include <sstream>
#include <numeric>
#include <algorithm>
#include <utility>
#include <cassert>
#include <iterator>

// tiny multi-precision integer class
class tinymp
{
	typedef unsigned int elem_type;
	typedef unsigned long long widen_type;
	typedef std::vector<elem_type> vector_type;
	vector_type v;
	bool nonneg;
	void normalize() {
		while(v.size() > 1 && v.back() == 0) v.pop_back();
		if(is_zero()) nonneg = true;
	}
	class offseter // minimal offset vector access
	{
		vector_type::const_iterator start;
		std::size_t sz;
		static elem_type zero;
		std::size_t offset;
	public:
		offseter(const std::vector<elem_type>& pv_, std::size_t offset_) : start(pv_.begin()), sz(pv_.size()), offset(offset_) {}
		offseter(vector_type::const_iterator start_, vector_type::const_iterator end, std::size_t offset_) : start(start_), sz(end - start_), offset(offset_) {}
		class riterator { // lexicographical_compare() requires InputIterator
			const offseter *p;
			std::size_t pos; // [0, p->sz + p->offset]: (p->sz + p->offset) points to p->start[p->sz - 1]
		public:
			typedef std::input_iterator_tag iterator_category;
			typedef elem_type value_type;
			typedef std::ptrdiff_t difference_type;
			typedef const elem_type* pointer;
			typedef const elem_type& reference;
			riterator(const offseter *p_, std::size_t pos_) : p(p_), pos(pos_) {}
			reference operator*() { return pos >= p->offset + 1 ? p->start[pos - p->offset - 1] : zero; }
			riterator& operator++() { --pos; return *this; }
			riterator operator++(int) { riterator r(*this); --pos; return std::move(r); }
			bool operator==(const riterator& other) const { return p == other.p && pos == other.pos; }
			bool operator!=(const riterator& other) const { return !(*this == other); }
		};
		const elem_type& operator[](std::size_t idx) const { return idx >= offset ? start[idx - offset] : zero; }
		riterator rbegin() const { return riterator(this, sz + offset); }
		riterator rend() const { return riterator(this, 0); }
		std::size_t size() const { return sz + offset; }
	};
	void addsub(vector_type::const_iterator begin, vector_type::const_iterator end, bool oneg, std::size_t offset = 0) {
		std::size_t sz = end - begin;
		offseter ov_(begin, end, offset);
		if(ov_.size() > v.size()) v.resize(ov_.size());
		if(nonneg == oneg) {
			elem_type carry = 0;
			for(std::size_t i = 0; i < v.size() - offset; ++i) {
				std::size_t idx = offset + i;
				elem_type new_carry;
				elem_type oval = idx < ov_.size() ? ov_[idx] : 0;
				if(std::numeric_limits<elem_type>::max() - oval < v[idx] + carry) {
					new_carry = 1;
				} else {
					new_carry = 0;
				}
				v[idx] += oval + carry;
				carry = new_carry;
				if( i >= sz && carry == 0) break;
			}
			if(carry) {
				v.push_back(1);
			}
		} else {
			bool sub = absless(ov_);
			offseter v_(v, 0); // iterator may be invalidated above
			offseter &lhs = sub ? ov_ : v_;
			offseter &rhs = sub ? v_ : ov_;
			// ensures v.size() == lhs.size() >= rhs.size()
			elem_type borrow = 0;
			for(std::size_t i = 0; i < v.size() - offset; ++i) {
				std::size_t idx = offset + i;
				elem_type new_borrow;
				elem_type rval = idx < rhs.size() ? rhs[idx] : 0;
				if(lhs[idx] < rval + borrow) {
					new_borrow = 1;
				} else {
					new_borrow = 0;
				}
				v[idx] = lhs[idx] - rval - borrow;
				borrow = new_borrow;
				if( i >= sz && borrow == 0) break;
			}
			assert(borrow == 0);
			normalize();
			nonneg = sub ? oneg : nonneg;
		}
	}
	void addsub(const std::vector<elem_type> &ov, bool oneg, std::size_t offset = 0) {
		addsub(ov.begin(), ov.end(), oneg, offset);
	}
	bool absless(const std::vector<elem_type> &ov, std::size_t offset = 0) const {
		return absless(offseter(ov.begin(), ov.end(), offset));
	}
	bool absless(const offseter &ov_) const {
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
	// FIXME: exception safety
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
	// Karatsuba algorithm
	tinymp mult(vector_type::const_iterator begin1, vector_type::const_iterator end1, vector_type::const_iterator begin2, vector_type::const_iterator end2) const {
		if(end1 - begin1 <= 1 && end2 - begin2 <= 1) {
			widen_type v1 = begin1 == end1 ? 0 : *begin1;
			widen_type v2 = begin2 == end2 ? 0 : *begin2;
			widen_type A = (v1 >> 16) * (v2 >> 16);
			widen_type B = ((v1 & 0xFFFF) + (v1 >> 16)) * ((v2 & 0xFFFF) + (v2 >> 16));
			widen_type C = (v1 & 0xFFFF) * (v2 & 0xFFFF);
			widen_type ret = (A << 32) + ((B - A - C) << 16) + C;
			tinymp r;
			r.v[0] = ret & 0xFFFFFFFF;
			r.v.push_back(ret >> 32);
			r.normalize();
			return std::move(r);
		} else {
			std::size_t len1 = end1 - begin1, len2 = end2 - begin2;
			std::size_t N = (std::max(len1, len2) + 1) / 2;
			auto A = mult(begin1 + std::min(N, len1), begin1 + std::min(2*N, len1), begin2 + std::min(N, len2), begin2 + std::min(2*N, len2));
			tinymp a(begin1, begin1 + std::min(N, len1));
			tinymp c(begin2, begin2 + std::min(N, len2));
			a.addsub(begin1 + std::min(N, len1), begin1 + std::min(2*N, len1), true, 0);
			c.addsub(begin2 + std::min(N, len2), begin2 + std::min(2*N, len2), true, 0);
			auto B = mult(a.v.begin(), a.v.end(), c.v.begin(), c.v.end());
			auto C = mult(begin1, begin1 + std::min(N, len1), begin2, begin2 + std::min(N, len2));
			tinymp r;
			r.addsub(A.v, true, N*2);
			r.addsub(B.v, true, N);
			r.addsub(A.v, false, N);
			r.addsub(C.v, false, N);
			r.addsub(C.v, true, 0);
			return std::move(r);
		}
	}
public:
	template<typename InIt>
	tinymp(InIt it1, InIt it2): v(it1, it2), nonneg(true) {
		if(v.size() == 0) v.push_back(0);
	}
	tinymp(elem_type val = 0): v(1, val), nonneg(true) {}
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
	std::pair<tinymp&, tinymp> div_(elem_type s) {
		widen_type unit = widen_type(1) << std::numeric_limits<elem_type>::digits;
		widen_type borrow = 0;
		for(std::size_t i = 0; i < v.size(); ++i) {
			auto temp = borrow * unit + v[v.size() - i - 1];
			v[v.size() - i - 1] = temp / s;
			borrow = temp % s;
		}
		normalize();
		tinymp remainder(borrow);
		if(!nonneg) remainder.flip_();
		return { *this, std::move(remainder) };
	}
	std::pair<tinymp, tinymp> div(elem_type s) const {
		return tinymp(*this).div_(s);
	}
	std::pair<tinymp&, tinymp> div_(const tinymp& other) {
		auto t = tinymp(*this).div(other);
		*this = std::move(t.first);
		return { *this, std::move(t.second) };
	}
	std::pair<tinymp, tinymp> div(const tinymp& other) const {
		// TODO: check Knuth algorithm
		if(!(absless(other))) {
			tinymp r;
			r.v.resize(v.size() - other.v.size() + 1);
			tinymp residual(*this);
			residual.nonneg = true;
			for(std::size_t i = 0; i < r.v.size(); ++i) {
				std::size_t idxr = r.v.size() - i - 1;
				if(residual.v.size() < idxr + other.v.size() || residual.absless(other.v, idxr)) continue;
				widen_type res = 0;
				if(residual.v.size() > idxr + other.v.size()) {
					res = widen_type(residual.v[idxr + other.v.size()]) << std::numeric_limits<elem_type>::digits;
				}
				res += residual.v[idxr + other.v.size() - 1];
				widen_type top = other.v.back();
				if(2 <= idxr + other.v.size() && other.v.size() >= 2) {
					std::size_t bres = std::numeric_limits<widen_type>::digits;
					while(bres > 0 && !((widen_type(1) << (bres-1)) & res)) --bres;
					std::size_t btop = std::numeric_limits<widen_type>::digits;
					while(btop > 0 && !((widen_type(1) << (btop-1)) & top)) --btop;
					std::size_t bits = std::min<widen_type>(32, std::numeric_limits<widen_type>::digits - std::max(bres, btop));
					if(bits) {
						res <<= bits;
						res += residual.v[idxr + other.v.size() - 2] >> (std::numeric_limits<elem_type>::digits - bits);
						top <<= bits;
						top += other.v[other.v.size() - 2] >> (std::numeric_limits<elem_type>::digits - bits);
					}
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
			if(!nonneg) residual.flip_();
			return { std::move(r), residual };
		} else {
			return { {}, *this };
		}
	}
	tinymp mult(const tinymp& other) const {
		// faster for 800len * 800len order
		return mult(v.begin(), v.end(), other.v.begin(), other.v.end());
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
	friend inline tinymp operator%(tinymp other, elem_type s) {
		return std::move(other.div_(s).second);
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
	friend inline tinymp stotmp(const std::string &s);
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
inline tinymp stotmp(const std::string &s) {
	return std::move(tinymp().from_chars(s.data(), s.size()));
}
