#pragma once

#include "config.hpp"
#include "random.hpp"

struct Key {
	i32 val;
	Key(): val(0) {}
	Key(i32 x): val(x) {}
	auto operator < (const Key &rhs) const -> bool {
		return val < rhs.val;
	}
};

Rng rng;

struct Value {
	char s[70];
	Value() {
		memset(s, 0, sizeof s);
		i32 len = rng(1, 70);
		for (i32 i = 0; i < len; ++i)
			if (auto r = rng(0, 2); r == 0) s[i] = rng('0', '9');
			else if (r == 1) s[i] = rng('a', 'z');
			else s[i] = rng('A', 'Z');
	}
	Value(const char *t): Value() { memcpy(s, t, strlen(t) * sizeof(char)); }
	Value(const str &t): Value() { memcpy(s, t.c_str(), t.length() * sizeof(char)); }
	auto operator < (const Value &rhs) const -> bool {
		for (i32 i = 0; i < 70 and s[i]; ++i)
			if (rhs.s[i] < s[i])
				return false;
			else if (rhs.s[i] > s[i])
				return true;
		return false;
	}
	auto operator == (const Value &rhs) const -> bool {
		for (i32 i = 0; i < 70; ++i)
			if (s[i] != rhs.s[i])
				return false;
		return true;
	}
	auto operator != (const Value &rhs) const -> bool { return not (*this == rhs); }
};
