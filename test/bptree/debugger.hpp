#pragma once

#include "config.hpp"
#include "bptree.hpp"

template <typename Key, typename Value>
struct debugger {
	using tree_t = bptree<Key, Value>;

	tree_t *tree;
	const str indent_base;

	debugger(): indent_base("\t\t") {}

	auto printLeaf(HardDisk::Record rec, i32 depth) -> void {
		str indent("");
		for (i32 i = 0; i < depth; ++i) indent += indent_base;

		if (rec.empty()) {
			printf("%s", indent.c_str()); puts("-------------------------------------------------------------------------------------------");
			printf("%s", indent.c_str()); puts("this node don't exist!");
			printf("%s", indent.c_str()); puts("-------------------------------------------------------------------------------------------");
			return;
		}

		tree_t::leaf_node *u = static_cast<tree_t::leaf_node*>(std::malloc(sizeof(tree_t::leaf_node)));
		rec.load(tree->indexfile, *u);

		printf("%s", indent.c_str()); puts("-------------------------------------------------------------------------------------------");
		printf("%s", indent.c_str()); printf("leaf_node: (offset = %lld)\n", rec.offset);
		printf("%s", indent.c_str()); printf("{ size = %lu, left = %lld, right = %lld }\n", u->size, u->left.offset, u->right.offset);
		printf("%s", indent.c_str()); printf("key = ("); for (i32 i = 0; i < i32(u->size); ++i) printf("[%d] = %d, ", i, u->key[i]); printf(")\n");
		printf("%s", indent.c_str()); printf("rec = ("); for (i32 i = 0; i < i32(u->size); ++i) printf("[%d] = %lld, ", i, u->rec[i].offset); printf(")\n");
		printf("%s", indent.c_str()); puts("-------------------------------------------------------------------------------------------");

		std::free(u);
	}

	auto printInternal(HardDisk::Record rec, i32 depth) -> void {
		str indent("");
		for (i32 i = 0; i < depth; ++i) indent += indent_base;

		if (rec.empty()) {
			printf("%s", indent.c_str()); puts("-------------------------------------------------------------------------------------------");
			printf("%s", indent.c_str()); puts("this node don't exist!");
			printf("%s", indent.c_str()); puts("-------------------------------------------------------------------------------------------");
			return;
		}

		tree_t::internal_node *u = static_cast<tree_t::internal_node*>(std::malloc(sizeof(tree_t::internal_node)));
		rec.load(tree->indexfile, *u);

		printf("%s", indent.c_str()); puts("-------------------------------------------------------------------------------------------");
		printf("%s", indent.c_str()); printf("internal_node: (offset = %lld)\n", rec.offset);
		printf("%s", indent.c_str()); printf("{ size = %lu, sub_is_leaf = %s }\n", u->size, u->sub_is_leaf ? "true" : "false");
		printf("%s", indent.c_str()); printf("key = ("); for (i32 i = 0; i < i32(u->size); ++i) printf("[%d] = %d, ", i, u->key[i]); printf(")\n");
		printf("%s", indent.c_str()); printf("sub = ("); for (i32 i = 0; i <= i32(u->size); ++i) printf("[%d] = %lld, ", i, u->sub[i].offset); printf(")\n");
		printf("%s", indent.c_str()); puts("-------------------------------------------------------------------------------------------");

		if (u->sub_is_leaf)
			for (i32 i = 0; i <= i32(u->size); ++i)
				printLeaf(u->sub[i], depth + 1);
		else for (i32 i = 0; i <= i32(u->size); ++i)
				printInternal(u->sub[i], depth + 1);

		std::free(u);
	}

	auto printTree(tree_t &__tree, const char *msg = "") -> void {
		tree = std::addressof(__tree);
		puts(msg);
		printf("FACTOR = %d\n", __tree.FACTOR);
		printInternal(tree->header.root, 0);
	}

	auto operator () (tree_t &__tree, const char *msg = "") -> void {
		printTree(__tree, msg);
	}
};
