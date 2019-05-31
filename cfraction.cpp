#include <iostream>
#include <vector>
#include "rational.cpp"

// FIXME: signed and unsigned
template<typename T>
struct cfraction
{
	cfraction(const rational<T> &val) {
		if(val.num() == 0) v.push_back(0);
		else {
			T n = val.num();
			T d = val.den();
			while(d != 0) {
				using std::swap;
				v.push_back(n / d);
				n = n % d;
				swap(n, d);
			}
		}
	}
	template<typename InputIterator>
	cfraction(InputIterator it1, InputIterator it2) : v(it1, it2) {
		if(v.size() == 0) v.push_back(0);
	}
	cfraction(std::initializer_list<int> l) : cfraction(l.begin(), l.end()) {
	}
	cfraction& to_alt() {
		if(v.back() == 1) {
			v.pop_back(); v.back() += 1;
		} else if(v.back() != 0) {
			v.back() -= 1; v.push_back(1);
		}
		return *this;
	}
	cfraction alt() const {
		cfraction ret(*this); ret.to_alt(); return ret;
	}
	operator rational<T>() const {
		using std::swap;
		T n = 0, d = 1;
		for(auto it = v.rbegin(); it != v.rend(); ++it) {
			n += *it * d;
			swap(n, d);
		}
		swap(n, d);
		return { n, d };
	}
	static cfraction best(const cfraction &v1, const cfraction &v2) { // FIXME: dumb logic
		auto r1 = rational<T>(v1);
		auto r2 = rational<T>(v2);
		if(r1 == r2) throw std::domain_error("requires non-empty interval");
		auto r = best_imp(v1, v2); auto rr = rational<T>(r);
		auto update = [&](const cfraction &vv1, const cfraction &vv2){ 
			auto r_ = best_imp(vv1, vv2); auto rr_ = rational<T>(r_);
			if(rr_ != r1 && rr_ != r2  && (rr == r1 || rr == r2 || rr.den() > rr_.den())) { r = r_; rr = rr_; }
		};
		update(v1, v2.alt());
		update(v1.alt(), v2);
		update(v1.alt(), v2.alt());
		return r;
	}
	cfraction best(const cfraction &v) const { return best(*this, v); }
	template<typename F>
	void foreach_approx(F f) const {
		cfraction app(v.begin(), v.begin()+1);
		for(std::size_t idx = 1; idx < v.size(); ++idx) {
			app.v.push_back(0);
			for(T rval = v[idx] / 2 + (v[idx] % 2 == 1 || !check_half(idx)); rval <= v[idx]; ++rval) {
				app.v[idx] = rval;
				if(f(app)) break;
			}
		}
	}
	rational<T> approx(T denom) const {
		rational<T> r;
		foreach_approx([&r,denom](const cfraction &val){
			auto t = rational<T>(val);
			if(t.den() > denom) return true;
			r = std::move(t);
			return false;
		});
		return r;
	}
	friend std::ostream& operator<<(std::ostream& os, const cfraction &v) {
		int n = 0;
		os << '[';
		for(auto val: v.v) { if(n) os << (n == 1 ? ';' : ','); os << val; if(n<2) ++n; }
		os << ']';
		return os;
	}
	friend bool operator==(const cfraction &v1, const cfraction &v2) {
		return v1.v == v2.v;
	}
private:
	std::size_t min_size() const { return v.size() - (v.size() != 1 &&  v.back() == 1); }
	static cfraction best_imp(const cfraction &v1, const cfraction &v2) {
		auto N = std::min(v1.v.size(), v2.v.size());
		auto e = std::mismatch(v1.v.begin(), v1.v.begin() + N, v2.v.begin());
		cfraction ret(0);
		ret.v.assign(v1.v.begin(), e.first);
		ret.v.push_back((e.first == v1.v.end() ? *e.second : e.second == v2.v.end() ? *e.first : std::min(*e.first, *e.second)) + 1);
		return ret;
	}
	bool check_half(std::size_t idx) const {
		cfraction v1(v.rend() - idx - 1, v.rend() - 1);
		cfraction v2(v.begin() + idx, v.end());
		return rational<T>(v1) > rational<T>(v2);
	}
	std::vector<T> v;
};
