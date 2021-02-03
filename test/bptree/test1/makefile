test: clean main
	time ./main 1000
	make clean_database
	time ./main 10000
	make clean_database
	time ./main 100000
	make clean_database
	time ./main 1000000
	make clean_database
	time ./main 10000000
	make clean_database
	make clean

%: %.cpp
	clang++ -Wall -Wextra -Wshadow -std=c++2a -stdlib=libc++ -O2 $^ -o $@ -I.. -I../../../src -I../../../src/bptree

clean: clean_database
	rm -rf main

clean_database:
	rm -rf data.bin index.bin

# $@：目标的名字
# $^：构造所需文件列表所有所有文件的名字
# $<：构造所需文件列表的第一个文件的名字
# $?：构造所需文件列表中更新过的文件
