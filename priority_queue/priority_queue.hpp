#ifndef SJTU_PRIORITY_QUEUE_HPP
#define SJTU_PRIORITY_QUEUE_HPP

#include <cstddef>
#include <functional>
#include "exceptions.hpp"

namespace sjtu {

template <typename T, class Compare = std::less<T>>
class priority_queue {
	using Self = priority_queue;
public:
	using value_type		= T;
	using reference			= value_type&;
	using const_reference	= const value_type&;
	using value_compare		= Compare;
	using size_type			= size_t;

private:
	struct HeapNode;

	auto clone(HeapNode *u) -> HeapNode*;
	auto remove(HeapNode *u) -> void;
	auto merge(HeapNode *lhs, HeapNode *rhs) -> HeapNode*;

	HeapNode *__root;
	size_type __size;
	value_compare __comp;

public:
	priority_queue(): __root(nullptr), __size(0), __comp() {}
	priority_queue(const Self &);

	~priority_queue() { remove(__root); }

	auto operator = (const Self &other) -> Self&;

	auto size() const -> size_type { return __size; }
	auto empty() const -> bool { return __root == nullptr; }

	auto top() const -> const_reference;

	auto push(const value_type &value) -> void;
	auto pop() -> void;

	auto merge(Self &other) -> void;
};


	template <typename T, class Compare>
	struct priority_queue<T, Compare>::HeapNode {
		size_type depth;
		HeapNode *ls, *rs;
		value_type value;

		HeapNode(const HeapNode &other) = default;
		HeapNode(HeapNode &&other): depth(other.depth), ls(other.ls), rs(other.rs), value(std::move(other.value)) {
			other.ls = other.rs = nullptr;
		}

		HeapNode(const value_type &_value = value_type())
			: depth(0), ls(nullptr), rs(nullptr), value(_value) {}
		HeapNode(size_type _depth, HeapNode *_ls, HeapNode *_rs, const value_type &_value)
			: depth(_depth), ls(_ls), rs(_rs), value(_value) { self_adjust(); }

		auto self_adjust() -> void {
			if (ls == nullptr or (rs != nullptr and ls->depth < rs->depth))
				std::swap(ls, rs);
			depth = (rs != nullptr ? rs->depth + 1 : 0);
		}

		~HeapNode() = default;
	};


/* impl priority_queue<T, Compare>::HeapNode { */

	template <typename T, class Compare>
	auto priority_queue<T, Compare>::clone(HeapNode *u) -> HeapNode* {
		return u != nullptr ? new HeapNode(u->depth, clone(u->ls), clone(u->rs), u->value) : nullptr;
	}

	template <typename T, class Compare>
	auto priority_queue<T, Compare>::remove(HeapNode *u) -> void {
		if (u != nullptr) remove(u->ls), remove(u->rs);
		delete u;
	}

	template <typename T, class Compare>
	auto priority_queue<T, Compare>::merge(HeapNode *lhs, HeapNode *rhs) -> HeapNode* {
		if (lhs == nullptr) return rhs;
		if (rhs == nullptr) return lhs;
		if (__comp(lhs->value, rhs->value)) std::swap(lhs, rhs);
		lhs->rs = merge(lhs->rs, rhs);
		lhs->self_adjust();
		return lhs;
	}

/* } */



/* impl priority_queue<T, Compare> { */

	template <typename T, class Compare>
	priority_queue<T, Compare>::priority_queue(const Self &other)
		: __root(clone(other.__root)), __size(other.__size), __comp(other.__comp) {}


	template <typename T, class Compare>
	auto priority_queue<T, Compare>::operator = (const Self &other) -> Self& {
		if (this == std::addressof(other)) return *this;
		remove(__root);
		__root = clone(other.__root);
		__size = other.__size;
		return *this;
	}

	template <typename T, class Compare>
	auto priority_queue<T, Compare>::top() const -> const_reference {
		if (__root == nullptr)
			throw container_is_empty();
		return __root->value;
	}

	template <typename T, class Compare>
	auto priority_queue<T, Compare>::push(const value_type &value) -> void {
		__root = merge(__root, new HeapNode(value));
		++__size;
	}

	template <typename T, class Compare>
	auto priority_queue<T, Compare>::pop() -> void {
		if (__root == nullptr)
			throw container_is_empty();
		auto tmp = __root;
		__root = merge(__root->ls, __root->rs);
		delete tmp;
		--__size;
	}

	template <typename T, class Compare>
	auto priority_queue<T, Compare>::merge(Self &other) -> void {
		__root = merge(__root, other.__root);
		__size += other.__size;
		other.__root = nullptr;
		other.__size = 0;
	}

/* } */

}

#endif
