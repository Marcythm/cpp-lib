#pragma once

#include "config.hpp"

struct Timer {
	clock_t time;
	auto start() -> void { time = clock(); }
	auto stop() -> clock_t { clock_t tmp = clock() - time; return time = clock(), tmp; }
};
