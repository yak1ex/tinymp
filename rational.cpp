#include <type_traits>
#include <cstdint>
#include <limits>
#include <iostream>

template<std::size_t Bits, bool Signed> struct int_type {};
template<> struct int_type<16, false> { typedef std::uint16_t type; };
template<> struct int_type<32, false> { typedef std::uint32_t type; };
template<> struct int_type<64, false> { typedef std::uint64_t type; };
template<> struct int_type<16, true> { typedef std::int16_t type; };
template<> struct int_type<32, true> { typedef std::int32_t type; };
template<> struct int_type<64, true> { typedef std::int64_t type; };
template<typename T, bool F> struct widen_imp {
	typedef std::numeric_limits<T> ltype;
	typedef typename int_type<2 * (ltype::digits + (ltype::is_signed != false)), ltype::is_signed>::type type;
};
template<typename T> struct widen_imp<T, false> { typedef T type; };
template<typename T> struct widen : widen_imp<T, std::numeric_limits<T>::is_specialized> {};
template<typename T, bool F> struct my_make_unsigned_imp { typedef typename std::make_unsigned<T>::type type; };
template<typename T> struct my_make_unsigned_imp<T, false> { typedef T type; };
template<typename T> struct my_make_unsigned : my_make_unsigned_imp<T, std::is_integral<T>::value> {};
template<typename T, bool F> struct my_make_signed_imp { typedef typename std::make_signed<T>::type type; };
template<typename T> struct my_make_signed_imp<T, false> { typedef T type; };
template<typename T> struct my_make_signed : my_make_signed_imp<T, std::is_integral<T>::value> {};

template<typename T>
struct rational_imp
{
	typedef typename widen<T>::type widen_type;
	typedef typename my_make_signed<T>::type numer_type;
	typedef typename widen<numer_type>::type wnumer_type;
	rational_imp(numer_type num = 0, T deno = 1) : numer(num), denom(deno) {} // FIXME: check 0 0
	template<typename U, typename = typename std::enable_if<std::is_constructible<T, U>::value>::type>
	rational_imp(const rational_imp<U>& v) : numer(v.num()), denom(v.den()) {}
	friend rational_imp operator+(const rational_imp &v1) { return v1; }
	friend rational_imp operator+(rational_imp &&v1) { return std::move(v1); }
	friend rational_imp operator-(const rational_imp &v1) { rational_imp r(v1); r.numer = -std::move(r.numer); return r; }
	friend rational_imp operator-(rational_imp &&v1) { v1.numer = -std::move(v1.numer); return std::move(v1); }
	friend rational_imp operator+(const rational_imp &v1, const rational_imp &v2) { rational_imp r(v1); r += v2; return r; }
	friend rational_imp operator-(const rational_imp &v1, const rational_imp &v2) { rational_imp r(v1); r -= v2; return r; }
	friend rational_imp operator*(const rational_imp &v1, const rational_imp &v2) { rational_imp r(v1); r *= v2; return r; }
	friend rational_imp operator/(const rational_imp &v1, const rational_imp &v2) { rational_imp r(v1); r /= v2; return r; }
	rational_imp& normalize() { auto v = gcd(safe_abs(numer), denom); if(v) { numer /= v; denom /= v; } }
	rational_imp& operator+=(const rational_imp& other) {
		auto n = static_cast<wnumer_type>(numer) * other.denom + static_cast<wnumer_type>(other.numer) * denom; // may overflow
		auto d = static_cast<widen_type>(denom) * other.denom;
		auto g = gcd(safe_abs(n), d);
		if(g == 0) g = 1;
		numer = n / g; denom = d / g; // may overflow
		return *this;
	}
	rational_imp& operator-=(const rational_imp& other) {
		auto n = static_cast<wnumer_type>(numer) * other.denom - static_cast<wnumer_type>(other.numer) * denom; // may overflow
		auto d = static_cast<widen_type>(denom) * other.denom;
		auto g = gcd(safe_abs(n), d);
		if(g == 0) g = 1;
		numer = n / g; denom = d / g; // may overflow
		return *this;
	}
	rational_imp& operator*=(const rational_imp& other) {
		auto n = static_cast<wnumer_type>(numer) * other.numer;
		auto d = static_cast<widen_type>(denom) * other.denom;
		auto g = gcd(safe_abs(n), d);
		if(g == 0) g = 1;
		numer = n / g; denom = d / g; // may overflow
		return *this;
	}
	rational_imp& operator/=(const rational_imp& other) { // naive
		auto n = static_cast<wnumer_type>(numer) * other.denom;
		auto d = static_cast<wnumer_type>(denom) * other.numer;
		auto g = gcd(safe_abs(n), safe_abs(d));
		if(g == 0) g = 1;
		numer = (d < 0 ? -n : n) / g; denom = (d < 0 ? -d : d) / g; // may overflow
		return *this;
	}
	const numer_type& num() const { return numer; }
	const T& den() const { return denom; }
	friend std::ostream& operator<<(std::ostream& os, const rational_imp& v) {
		return os << v.numer << '/' << v.denom;
	}
	friend std::istream& operator<<(std::istream& is, rational_imp& v) {
		char c;
		return is >> v.numer >> c >> v.denom;
	}
private:
	numer_type numer;
	T denom;
	template<typename U>
	static inline U gcd(U m, U n)
	{
		if(m < n) std::swap(m, n);
		while(n != 0) {
			U r = m % n;
			m = std::move(n);
			n = std::move(r);
		}
		return std::move(m);
	}
	template<typename U>
	static inline typename my_make_unsigned<U>::type safe_abs(const U& v) {
		return v < 0 ? -static_cast<typename std::make_unsigned<U>::type>(v) : v;
	}
};
template<typename T, typename U>
inline bool operator<(const rational_imp<T> &v1, const rational_imp<U> &v2) {
	typedef typename std::common_type<typename rational_imp<T>::wnumer_type, typename rational_imp<U>::wnumer_type>::type wnumer_type;
	if(v1.num() < 0 && 0 < v2.num()) return true;
	if(v1.num() > 0 && 0 < v2.num()) return false;
	return static_cast<wnumer_type>(v1.num()) * v2.den() < static_cast<wnumer_type>(v2.num()) * v1.den();
}
template<typename T, typename U> inline bool operator>(const rational_imp<T> &v1, const rational_imp<U> &v2) { return v2 < v1; }
template<typename T, typename U> inline bool operator<=(const rational_imp<T> &v1, const rational_imp<U> &v2) { return !(v2 < v1); }
template<typename T, typename U> inline bool operator>=(const rational_imp<T> &v1, const rational_imp<U> &v2) { return !(v1 < v2); }
template<typename T, typename U> inline bool operator==(const rational_imp<T> &v1, const rational_imp<U> &v2) { return v1 <= v2 && v1 >= v2; }
template<typename T, typename U> inline bool operator!=(const rational_imp<T> &v1, const rational_imp<U> &v2) { return v1 < v2 || v1 > v2; }

template<typename T>
struct rational : rational_imp<typename my_make_unsigned<T>::type>
{
	typedef rational_imp<typename my_make_unsigned<T>::type> base_type;
	using base_type::base_type;
	rational(const base_type &base) : base_type(base) {}
	rational(base_type &&base) : base_type(base) {}
};
