using i32 = int;
using i64 = long long;

template <const i32 p>
class m32 {
private:
	i32 v;

public:
	m32(): v(0) {}
	m32(const i32 &val): v(val % p) {}
	m32(const m32 &rhs): v(rhs.v) {}

	operator i32&() { return v; }
	auto operator & () -> i32* { return &v; }

	auto operator = (const m32 &rhs) -> m32& { return v = rhs.v, *this; }

	auto operator + (const m32 &rhs) const -> m32 { return m32((v + rhs.v < p) ? (v + rhs.v) : (v + rhs.v - p)); }
	auto operator - (const m32 &rhs) const -> m32 { return m32((v - rhs.v < 0) ? (v - rhs.v + p) : (v - rhs.v)); }
	auto operator * (const m32 &rhs) const -> m32 { return m32(i64(v) * rhs.v % p); }
	auto operator / (const m32 &rhs) const -> m32 { return operator*(rhs.inv()); }
	auto operator ^ (const m32 &rhs) const -> m32 {
		m32 ans(1), pw(1);
		for (i32 n = rhs.v; n; n >>= 1, pw *= v)
			if (n & 1) ans *= pw;
		return ans;
	}
	auto inv() const -> m32 { return operator^(p - 2); }

	auto operator += (const m32 &rhs) -> m32& { if ((v += rhs.v) >= p) v -= p; return *this; }
	auto operator -= (const m32 &rhs) -> m32& { if ((v -= rhs.v) < 0) v += p; return *this; }
	auto operator *= (const m32 &rhs) -> m32& { v = i64(v) * rhs.v % p; return *this; }
	auto operator /= (const m32 &rhs) -> m32& { return *this = operator*(rhs.inv());}
	auto operator ^= (const m32 &rhs) -> m32& { return *this = operator^(rhs); }

	template <typename T> auto operator + (const T &rhs) const -> m32 { return operator+(m32(rhs)); }
	template <typename T> auto operator - (const T &rhs) const -> m32 { return operator-(m32(rhs)); }
	template <typename T> auto operator * (const T &rhs) const -> m32 { return operator*(m32(rhs)); }
	template <typename T> auto operator / (const T &rhs) const -> m32 { return operator/(m32(rhs)); }

	template <typename T> auto operator += (const T &rhs) -> m32& { return operator+=(m32(rhs)); }
	template <typename T> auto operator -= (const T &rhs) -> m32& { return operator-=(m32(rhs)); }
	template <typename T> auto operator *= (const T &rhs) -> m32& { return operator*=(m32(rhs)); }
	template <typename T> auto operator /= (const T &rhs) -> m32& { return operator/=(m32(rhs)); }

	template <typename T> friend auto operator + (const T &lhs, const m32 &rhs) -> m32 { return m32(lhs) + rhs; }
	template <typename T> friend auto operator - (const T &lhs, const m32 &rhs) -> m32 { return m32(lhs) - rhs; }
	template <typename T> friend auto operator * (const T &lhs, const m32 &rhs) -> m32 { return m32(lhs) * rhs; }
	template <typename T> friend auto operator / (const T &lhs, const m32 &rhs) -> m32 { return m32(lhs) / rhs; }

	template <typename T> friend auto operator += (T &lhs, const m32 &rhs) -> T& { return lhs = m32(lhs) + rhs; }
	template <typename T> friend auto operator -= (T &lhs, const m32 &rhs) -> T& { return lhs = m32(lhs) - rhs; }
	template <typename T> friend auto operator *= (T &lhs, const m32 &rhs) -> T& { return lhs = m32(lhs) * rhs; }
	template <typename T> friend auto operator /= (T &lhs, const m32 &rhs) -> T& { return lhs = m32(lhs) / rhs; }
};
