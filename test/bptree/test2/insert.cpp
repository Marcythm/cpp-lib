#include "config.hpp"
#include "bptree.hpp"
#include "timer.hpp"
#include "random.hpp"
// #include "debugger.hpp"
#include "types.hpp"

using namespace __cpplib::__config;

using tree_t = __cpplib::bptree<Key, Value>;
tree_t tree;

auto main(i32, const char **argv) -> i32 {
	const i32 num = std::stoi(argv[1]);
	printf("num = %d\n", num);

	Key *keys = new Key[num];
	Value *vals = new Value[num];

	for (i32 i = 0; i < num; ++i) keys[i] = i;
	std::shuffle(keys, keys + num, std::mt19937());

	std::FILE *input = fopen("input", "w+b");
	fwrite(keys, sizeof(Key), num, input);
	fwrite(vals, sizeof(Value), num, input);
	fclose(input);

	Timer clk;
	clk.start();
	for (i32 i = 0; i < num; ++i) tree.insert(keys[i], vals[i]);
	printf("insert done, time = %.2lf\n", clk.stop() / f64(CLOCKS_PER_SEC));

	delete[] keys;
	delete[] vals;

	return 0;
}
