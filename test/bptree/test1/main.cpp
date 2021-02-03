#include "config.hpp"
#include "bptree.hpp"
#include "timer.hpp"
#include "random.hpp"
// #include "debugger.hpp"
#include "types.hpp"


using tree_t = bptree<Key, Value>;
tree_t tree;
// debugger<Key, Value> dbg;

auto main(i32, const char **argv) -> i32 {
	const i32 num = std::stoi(argv[1]);
	printf("num = %d\n", num);

	Key *keys = new Key[num];
	Value *vals = new Value[num];

	for (i32 i = 0; i < num; ++i) keys[i] = i;
	std::shuffle(keys, keys + num, std::mt19937());

	Timer clk;
	clk.start();
	for (i32 i = 0; i < num; ++i) /* try */ {
		tree.insert(keys[i], vals[i]);
		// printf("\n\n\n\nafter insert %d:\n", keys.back());
		// dbg(tree);
	} /* catch (const char *msg) {
		printf("exception caught! %s\n", msg);
		// dbg(tree);
		return 0;
	} */
	printf("insert done, time = %.2lf\n", clk.stop() / f64(CLOCKS_PER_SEC));
	// dbg(tree);

	for (i32 i = 0; i < num; ++i) {
		if ((Value)(*tree.find(keys[i])) != vals[i]) {
			printf("wrong!");
			exit(0);
		}
	}
	printf("find done, time = %.2lf\n", clk.stop() / f64(CLOCKS_PER_SEC));

	for (i32 i = 0; i < num; ++i) {
		if (tree.value(keys[i]) != vals[i]) {
			printf("wrong!");
			exit(0);
		}
	}
	printf("value done, time = %.2lf\n", clk.stop() / f64(CLOCKS_PER_SEC));

	for (i32 i = 0; i < num; ++i) /* try */ {
		tree.erase(keys[i]);
		// printf("\n\n\n\nafter erase %d:\n", key);
		// dbg(tree);
	} /* catch (const char *msg) {
		printf("exception caught! %s\n", msg);
		// dbg(tree);
		return 0;
	} */
	printf("erase done, time = %.2lf\n", clk.stop() / f64(CLOCKS_PER_SEC));

	delete[] keys;
	delete[] vals;

	return 0;
}


/*
	without -O2
		❯ time ./main 1000
			num = 1000
			insert done, time = 0.02
			find done, time = 0.01
			value done, time = 0.01
			erase done, time = 0.01
			./main 1000  0.03s user 0.02s system 93% cpu 0.058 total
		❯ time ./main 10000
			num = 10000
			insert done, time = 0.15
			find done, time = 0.12
			value done, time = 0.10
			erase done, time = 0.09
			./main 10000  0.30s user 0.18s system 99% cpu 0.481 total
		❯ time ./main 100000
			num = 100000
			insert done, time = 1.64
			find done, time = 1.36
			value done, time = 0.98
			erase done, time = 0.80
			./main 100000  2.97s user 1.97s system 99% cpu 4.951 total
		❯ time ./main 1000000
			num = 1000000
			insert done, time = 19.52
			find done, time = 16.65
			value done, time = 12.06
			erase done, time = 8.68
			./main 1000000  34.11s user 24.31s system 99% cpu 58.548 total
		❯ time ./main 10000000
			num = 10000000
			insert done, time = 261.57
			find done, time = 188.37
			value done, time = 143.69
			erase done, time = 99.91
			./main 10000000  370.58s user 338.59s system 99% cpu 11:53.71 total
 */

/*
	under -O2
		time ./main 1000
			num = 1000
			insert done, time = 0.01
			find done, time = 0.01
			value done, time = 0.01
			erase done, time = 0.01
			0.03 real         0.01 user         0.01 sys
		time ./main 10000
			num = 10000
			insert done, time = 0.10
			find done, time = 0.11
			value done, time = 0.09
			erase done, time = 0.09
			0.39 real         0.20 user         0.18 sys
		time ./main 100000
			num = 100000
			insert done, time = 1.13
			find done, time = 1.01
			value done, time = 0.81
			erase done, time = 1.06
			4.12 real         2.03 user         2.06 sys
		time ./main 1000000
			num = 1000000
			insert done, time = 14.46
			find done, time = 15.80
			value done, time = 11.81
			erase done, time = 13.30
			56.71 real        24.90 user        31.35 sys
		time ./main 10000000
			num = 10000000
			insert done, time = 185.77
			find done, time = 165.35
			value done, time = 122.73
			erase done, time = 146.98
			634.32 real       256.24 user       373.69 sys
 */
