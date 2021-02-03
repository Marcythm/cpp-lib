#pragma once

#include "config.hpp"

using namespace __cpplib::__config;

struct Rng {
	std::random_device seed;
	std::mt19937 rng;

	Rng(): rng(seed()) {}

	auto operator () (i32 l, i32 r) -> i32 {
		return rng() % (r - l + 1) + l;
	}
};
