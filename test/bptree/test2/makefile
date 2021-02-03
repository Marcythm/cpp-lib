test: clean insert find value erase
	./insert 1000; ./find 1000; ./value 1000; ./erase 1000;
	make clean_database
	./insert 10000; ./find 10000; ./value 10000; ./erase 10000;
	make clean_database
	./insert 100000; ./find 100000; ./value 100000; ./erase 100000;
	make clean_database
	./insert 1000000; ./find 1000000; ./value 1000000; ./erase 1000000;
	make clean_database
	./insert 10000000; ./find 10000000; ./value 10000000; ./erase 10000000;
	make clean_database
	make clean

%: %.cpp
	clang++ -Wall -Wextra -Wshadow -std=c++2a -stdlib=libc++ -O2 $^ -o $@ -I.. -I../../../src -I../../../src/bptree

clean: clean_database
	rm -rf insert find value erase input

clean_database:
	rm -rf data.bin index.bin

# $@：目标的名字
# $^：构造所需文件列表所有所有文件的名字
# $<：构造所需文件列表的第一个文件的名字
# $?：构造所需文件列表中更新过的文件
