#!/bin/zsh

for dir in `ls`
do
	if [ ! -d $dir ]; then;
		continue
	fi
	cd $dir
	clang++ code.cpp -std=c++2a -fsanitize=address -fsanitize=undefined -g -I.. -I../.. -o code -DDEBUG
	if [ $? -ne 0 ]; then;
		echo "\033[31mcompile error!\033[0m"
		break
	fi
	./code > output
	if [ $? -ne 0 ]; then;
		echo "\033[31mruntime error!\033[0m"
		break
	fi
	diff -w output answer.txt
	if [ $? -ne 0 ]; then;
		echo "\033[31mWrong Answer!\033[0m"
		break
	else
		echo "\033[32mtest $dir Accepted!\033[0m"
		rm -rf code output code.dSYM
	fi
	cd ..
done
