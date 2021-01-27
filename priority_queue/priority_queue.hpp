#ifndef SJTU_PRIORITY_QUEUE_HPP
#define SJTU_PRIORITY_QUEUE_HPP

#include <cstddef>
#include <functional>
#include "exceptions.hpp"
#include "utility.hpp"

namespace sjtu {

template <typename T, class Compare = std::less<T>>
class priority_queue {
public:
	using value_type		=	T;
	using reference			=	value_type&;
	using const_reference	=	const value_type&;
	using size_type			=	size_t;

private:

	struct HeapNode {
		size_type depth;
		HeapNode *ls, *rs;
		value_type value;

		HeapNode(const HeapNode &rhs) = default;
		HeapNode(HeapNode &&rhs): depth(rhs.depth), ls(rhs.ls), rs(rhs.rs), value(std::move(rhs.value)) {
			rhs.ls = rhs.rs = nullptr;
		}

		HeapNode(const value_type &_value = value_type())
			: depth(0), ls(nullptr), rs(nullptr), value(_value) {}
		HeapNode(size_type _depth, HeapNode *_ls, HeapNode *_rs, const value_type &_value)
			: depth(_depth), ls(_ls), rs(_rs), value(_value) { self_adjust(); }

		void self_adjust() {
			if (ls == nullptr or (rs != nullptr and ls->depth < rs->depth))
				std::swap(ls, rs);
			depth = (rs != nullptr ? rs->depth + 1 : 0);
		}

		~HeapNode() = default;
	};

	HeapNode* clone(HeapNode *u) {
		return u != nullptr ? new HeapNode(u->depth, clone(u->ls), clone(u->rs), u->value) : nullptr;
	}
	void remove(HeapNode *u) {
		if (u != nullptr) {
			remove(u->ls);
			remove(u->rs);
			delete u;
		}
	}
	HeapNode* merge(HeapNode *lhs, HeapNode *rhs) {
		if (lhs == nullptr) return rhs;
		if (rhs == nullptr) return lhs;
		if (__comp(lhs->value, rhs->value)) std::swap(lhs, rhs);
		lhs->rs = merge(lhs->rs, rhs);
		lhs->self_adjust();
		return lhs;
	}

	HeapNode *__root;
	size_type __size;
	Compare __comp;

public:
	priority_queue(): __root(nullptr), __size(0), __comp(Compare()) {}
	priority_queue(const priority_queue &other)
		: __root(clone(rhs.__root)), __size(rhs.__size), __comp(other.__comp) {}

	~priority_queue() { remove(__root); }

	priority_queue &operator=(const priority_queue &other) {
		if (this == &other) return *this;
		remove(__root);
		__root = clone(other.__root);
		__size = other.__size;
		return *this;
	}

	const_reference top() const {
		if (__root == nullptr)
			throw container_is_empty();
		return __root->value;
	}

	void push(const value_type &value) {
		__root = merge(__root, new Node(value));
		++__size;
	}

	void pop() {
		if (__root == nullptr)
			throw container_is_empty();
		auto tmp = __root;
		__root = merge(__root->ls, __root->rs);
		delete tmp;
		--__size;
	}

	size_type size() const { return __size; }

	bool empty() const { return root == nullptr; }

	void merge(priority_queue &other) {
		__root = merge(__root, other.__root);
		__size += other.__size;
		other.__root = nullptr;
		other.__size = 0;
	}
};

}

#endif
