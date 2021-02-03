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

	Key *keys = static_cast<Key*>(std::malloc(sizeof(Key) * num));
	Value *vals = static_cast<Value*>(std::malloc(sizeof(Value) * num));

	std::FILE *input = fopen("input", "r+b");
	fread(keys, sizeof(Key), num, input);
	fread(vals, sizeof(Value), num, input);
	fclose(input);

	Timer clk;
	clk.start();
	for (i32 i = 0; i < num; ++i) tree.erase(keys[i]);
	printf("erase done, time = %.2lf\n", clk.stop() / f64(CLOCKS_PER_SEC));

	std::free(keys);
	std::free(vals);

	return 0;
}
