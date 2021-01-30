#ifndef SJTU_DEQUE_HPP
#define SJTU_DEQUE_HPP

#include "exceptions.hpp"

#include <cstddef>

namespace sjtu {

#define STLite_NOEXCEPT

template <typename T1, typename T2>
class pair {
	using Self = pair;
public:
	using first_type	= T1;
	using second_type	= T2;

	T1 first;
	T2 second;

	constexpr pair(): first(), second() { }
	constexpr pair(const T1 &x, const T2 &y): first(x), second(y) { }
	template <typename U1, typename U2>
		constexpr pair(const U1 &x, const U2 &y)
			: first(x), second(y) { }
	// template <typename U1, typename U2>
	// 	constexpr pair(U1 &&x, U2 &&y)
	// 		: first(std::move(x)), second(std::move(y)) { }

	template <typename U1, typename U2>
		constexpr pair(const pair<U1, U2> &other)
			: first(other.first), second(other.second) { }
	// template <typename U1, typename U2>
	// 	constexpr pair(pair<U1, U2> &&other)
	// 		: first(std::move(other.first)), second(std::move(other.second)) { }

	template <typename U1, typename U2>
		constexpr auto operator = (const pair<U1, U2> &other) -> Self&
			{ first = other.first; second = other.second; return *this; }
	// template <typename U1, typename U2>
	// 	constexpr auto operator = (pair<U1, U2> &&other) -> Self&
	// 		{ first = std::move(other.first), second = std::move(other.second); return *this; }
};


template <typename T>
class deque {
	using Self = deque;
public:
	using value_type		= T;
	using reference			= value_type&;
	using const_reference	= const value_type&;
	using pointer			= value_type*;
	using const_pointer		= const value_type*;
	using size_type			= size_t;
	using difference_type	= int;

private:
	struct Block;

	size_type __size;
	Block *__data;

public:
	class iterator;
	class const_iterator;

	deque();
	deque(Self &&);
	deque(const Self &);

	~deque() { clear(); delete __data; }

	auto operator = (Self &&) -> Self&;
	auto operator = (const Self &) -> Self&;

	/**
	 * access specified element with bounds checking
	 * throw index_out_of_bound if out of bound.
	 */
	auto at(const size_type &) -> reference;
	auto at(const size_type &) const -> const_reference;
	auto operator[](const size_t &loc) -> reference { return at(loc); }
	auto operator[](const size_t &loc) const -> const_reference { return at(loc); }

	/**
	 * access the first element
	 * throw container_is_empty when the container is empty.
	 */
	auto front() -> reference;
	auto front() const -> const_reference;

	/**
	 * access the last element
	 * throw container_is_empty when the container is empty.
	 */
	auto back() -> reference;
	auto back() const -> const_reference;

	auto begin() -> iterator { return iterator(0, {__data->succ, __data->succ->begin()}, this); }
	auto cbegin() const -> const_iterator { return const_iterator(0, {__data->succ, __data->succ->cbegin()}, this); }

	auto end() -> iterator { return iterator(size(), {__data->prev, __data->prev->end()}, this); }
	auto cend() const -> const_iterator { return const_iterator(size(), {__data->prev, __data->prev->cend()}, this); }

	auto empty() const -> bool { return __size == 0; }
	auto size() const -> size_type { return __size; }

	auto clear() -> void;

	/**
	 * inserts elements at the specified locat on in the container.
	 * inserts value before loc
	 * returns an iterator pointing to the inserted value
	 *     throw if the iterator is invalid or it point to a wrong place.
	 */
	auto insert(iterator, const value_type &) -> iterator;

	/**
	 * removes specified element at loc.
	 * removes the element at loc.
	 * returns an iterator pointing to the following element, if loc pointing to the last element, end() will be returned.
	 * throw if the container is empty, the iterator is invalid or it points to a wrong place.
	 */
	auto erase(iterator loc) -> iterator;

	/**
	 * inserts an element to the beginning.
	 */
	auto push_front(const value_type &value) -> void { __data->succ->push_front(value); ++__size; }

	/**
	 * adds an element to the end
	 */
	auto push_back(const value_type &value) -> void { __data->prev->push_back(value); ++__size; }

	/**
	 * removes the first element.
	 *     throw when the container is empty.
	 */
	auto pop_front() -> void;

	/**
	 * removes the last element
	 *     throw when the container is empty.
	 */
	auto pop_back() -> void;
};









	template <typename T>
	struct deque<T>::Block {
		static constexpr size_type CONTENT_LIMIT	= 1000;
		static constexpr size_type HALF_BLOCK_SIZE	= 1500 + 15;
		static constexpr size_type BLOCK_SIZE		= CONTENT_LIMIT * 3 + 30;

		using Self = Block;
		class noinit_tag {};

		Self *prev, *succ;
		pointer *__data, *__beg, *__end;

		Block();
		Block(noinit_tag);
		Block(pointer *, size_type);
		Block(const Self &);
		~Block();

		auto clone() const -> Self*;
		auto data() const -> pointer* { return __beg; }
		auto size() const -> size_type { return __end - __beg; }
		auto clear() -> void { __beg = __end = __data + HALF_BLOCK_SIZE; }
		auto empty() const -> bool { return __beg == __end; }
		auto cap_back() const -> size_type { return __data + BLOCK_SIZE - __end; }
		auto ishead() const -> bool { return __data == nullptr; }

		auto begin() -> pointer* { return __beg; }
		auto cbegin() const -> const pointer* { return __beg; }
		auto end() -> pointer* { return __end; }
		auto cend() const -> const pointer* { return __end; }

		auto append_unchecked(pointer *, size_type, bool) -> void;
		auto append(pointer *, size_type, bool) -> void;
		auto move_data_in_block(pointer *) -> void;
		auto reserve(size_type) -> void;

		auto link(Self *) -> Self*;
		auto cut() -> Self*;
		auto merge() -> Self*;
		auto split(size_type) -> Self*;
		auto adjust() -> void;

		auto push_front(const value_type &value) -> void { *--__beg = new value_type(value); adjust(); }
		auto push_back(const value_type &value) -> void { *__end++ = new value_type(value); adjust(); }
		auto pop_front() -> void;
		auto pop_back() -> void;

		auto insert(size_type, const value_type &) -> void;
		auto erase(size_type) -> void;

		auto front() -> reference;
		auto front() const -> const_reference;
		auto back() -> reference;
		auto back() const -> const_reference;

		auto at(size_type) -> reference;
		auto at(size_type) const -> const_reference;

		/* special version for iterator */
		auto at_spec(size_type) -> pair<Self*, pointer*>;
		auto at_spec(size_type) const -> pair<const Self*, const pointer*>;
	};

/* impl deque<T>::Block { */

	template <typename T>
	deque<T>::Block::Block()
		: prev(this), succ(this),
			__data(new value_type*[BLOCK_SIZE]),
				__beg(__data + HALF_BLOCK_SIZE), __end(__beg) { }

	template <typename T>
	deque<T>::Block::Block(noinit_tag)
		: prev(this), succ(this), __data(nullptr), __beg(nullptr), __end(nullptr) { }

	template <typename T>
	deque<T>::Block::Block(pointer *src, size_type n): Block() {
		__beg = __end = __data + HALF_BLOCK_SIZE - n / 2;
		append_unchecked(src, n, true);
	}

	template <typename T>
	deque<T>::Block::Block(const Self &other)
		: prev(this), succ(this),
			__data(other.__data != nullptr ? new value_type*[BLOCK_SIZE] : nullptr),
				__beg(nullptr), __end(nullptr) {
					if (other.__data != nullptr) {
						__beg = __end = __data + HALF_BLOCK_SIZE - other.size() / 2;
						append_unchecked(other.data(), other.size(), false);
					}
				}

	template <typename T>
	deque<T>::Block::~Block() {
		for (auto p = __beg; p != __end; ++p) delete *p;
		delete[] __data;
	}

	template <typename T>
	auto deque<T>::Block::clone() const -> Self* {
		if (ishead()) {
			Self *target = new Block(noinit_tag());
			for (Self *p = succ; not (p->ishead()); p = p->succ)
				target->prev->link(new Block(*p));
			return target;
		} else return new Block(*this);
	}


	template <typename T>
	auto deque<T>::Block::append_unchecked(pointer *__src, size_type n, bool move) -> void {
		if (move) for (; n > 0; --n) *__end++ = *__src++;
		else for (; n > 0; --n)
				*__end++ = new value_type(**__src++);
	}

	template <typename T>
	auto deque<T>::Block::append(pointer *__src, size_type n, bool move) -> void {
		for (; n > 0; ) {
			auto step = std::min(n, cap_back());
			append_unchecked(__src, step, move);
			adjust();
			__src += step; n -= step;
		}
	}

	template <typename T>
	auto deque<T>::Block::move_data_in_block(pointer *__dst) -> void {
#ifndef STLite_NOEXCEPT
		if (__dst < __data or __data + BLOCK_SIZE <= __dst)
			throw "call Block::move_data_in_block(): __dst not in block";
#endif
		if (__beg < __dst) {
			auto o_beg = __beg, o_end = __end;
			__beg = __end = __dst + size();
			for (; o_beg != o_end; )
				*--__beg = *--o_end;
		} else if (__beg > __dst) {
			auto o_beg = __beg, o_end = __end;
			__beg = __end = __dst;
			for (; o_beg != o_end; )
				*__end++ = *o_beg++;
		}
	}

	template <typename T>
	auto deque<T>::Block::reserve(size_type __size) -> void {
		if (__size <= size()) return;
#ifndef STLite_NOEXCEPT
		if (__size > BLOCK_SIZE)
			throw "call Block::reserve(): reserve size too large";
#endif
		move_data_in_block(__data + HALF_BLOCK_SIZE - __size / 2);
	}


	template <typename T>
	auto deque<T>::Block::link(Self *target) -> Self* {
#ifndef STLite_NOEXCEPT
		if (target == nullptr)
			throw "call Block::link() with a null pointer: this will cause original successor lost!";
#endif
		Self *s = this, *u = target, *v = target->prev, *t = succ;
		s->succ = u; u->prev = s;
		v->succ = t; t->prev = v;
		return target;
	}

	template <typename T>
	auto deque<T>::Block::cut() -> Self* {
#ifndef STLite_NOEXCEPT
		if (succ->ishead() or (prev == this and succ == this))
			throw "call Block::cut() on a block without succ block: this will cut itself";
#endif
		Self *target = succ, *u = target->succ;
		succ = u; u->prev = this;
		target->prev = target->succ = target;
		return target;
	}

	template <typename T>
	auto deque<T>::Block::merge() -> Self* {
#ifndef STLite_NOEXCEPT
		if (succ->ishead())
			throw "call Block::merge() on a block without succ block";
#endif
		Self *target = cut();
		append(target->data(), target->size(), true);
		target->clear();
		delete target;
		return this;
	}

	template <typename T>
	auto deque<T>::Block::split(size_type loc) -> Self* {
#ifndef STLite_NOEXCEPT
		if (loc > size())
			throw "call Block::split() with argument loc too big";
#endif
		link(new Block(__beg + loc, size() - loc));
		__end = __beg + loc;
		return succ;
	}

	template <typename T>
	auto deque<T>::Block::adjust() -> void {
		if (ishead()) return;
		if (not (succ->ishead()) and size() + succ->size() < CONTENT_LIMIT) {
			reserve(size() + succ->size());
			merge();
		}
		if (size() >= CONTENT_LIMIT * 1.5)
			split(size() / 2);
		if (__beg == __data or __end == __data + BLOCK_SIZE)
			move_data_in_block(__data + HALF_BLOCK_SIZE - size() / 2);
		if (not (prev->ishead()) and prev->size() + size() < CONTENT_LIMIT) {
			prev->reserve(prev->size() + size());
			prev->merge();
			return;
		}
		if (empty() and not (prev->ishead() and succ->ishead())) delete prev->cut();
	}

	template <typename T>
	auto deque<T>::Block::pop_front() -> void {
#ifndef STLite_NOEXCEPT
		if (empty())
			throw "call Block::pop_front() on an empty block";
#endif
		delete *__beg++;
		adjust();
	}

	template <typename T>
	auto deque<T>::Block::pop_back() -> void {
#ifndef STLite_NOEXCEPT
		if (empty())
			throw "call Block::pop_back() on an empty block";
#endif
		delete *--__end;
		adjust();
	}


	template <typename T>
	auto deque<T>::Block::insert(size_type loc, const value_type &value) -> void {
		if (loc > size()) {
#ifndef STLite_NOEXCEPT
			if (succ->ishead())
				throw "in Block::insert(): argument loc too big";
#endif
			return succ->insert(loc - size(), value);
		}

		if (loc * 2 < size()) {
			for (auto p = __beg; p != __beg + loc; ++p)
				*(p - 1) = *p;
			--__beg;
		} else {
			for (auto p = __end; p != __beg + loc; --p)
				*p = *(p - 1);
			__end++;
		}
		__beg[loc] = new value_type(value);

		adjust();
	}

	template <typename T>
	auto deque<T>::Block::erase(size_type loc) -> void {
		if (loc >= size()) {
#ifndef STLite_NOEXCEPT
			if (succ->ishead())
				throw "in Block::erase(): argument loc too big";
#endif
			return succ->erase(loc - size());
		}

		delete __beg[loc];
		if (loc * 2 < size()) {
			for (auto p = __beg + loc; p != __beg; --p)
				*p = *(p - 1);
			__beg++;
		} else {
			--__end;
			for (auto p = __beg + loc; p != __end; ++p)
				*p = *(p + 1);
		}

		adjust();
	}

	template <typename T>
	auto deque<T>::Block::front() -> reference {
#ifndef STLite_NOEXCEPT
		if (empty()) throw "call Block::front() on an empty block";
#endif
		return *__beg[0];
	}

	template <typename T>
	auto deque<T>::Block::front() const -> const_reference {
#ifndef STLite_NOEXCEPT
		if (empty()) throw "call Block::front() on an empty block";
#endif
		return *__beg[0];
	}

	template <typename T>
	auto deque<T>::Block::back() -> reference {
#ifndef STLite_NOEXCEPT
		if (empty()) throw "call Block::back() on an empty block";
#endif
		return *__end[-1];
	}

	template <typename T>
	auto deque<T>::Block::back() const -> const_reference {
#ifndef STLite_NOEXCEPT
		if (empty()) throw "call Block::back() on an empty block";
#endif
		return *__end[-1];
	}


	template <typename T>
	auto deque<T>::Block::at(size_type loc) -> reference {
		if (loc >= size()) {
#ifndef STLite_NOEXCEPT
			if (succ->ishead())
				throw "in Block::at(): argument loc too big";
#endif
			return succ->at(loc - size());
		}
		return *__beg[loc];
	}

	template <typename T>
	auto deque<T>::Block::at(size_type loc) const -> const_reference {
		if (loc >= size()) {
#ifndef STLite_NOEXCEPT
			if (succ->ishead())
				throw "in Block::at(): argument loc too big";
#endif
			return succ->at(loc - size());
		}
		return *__beg[loc];
	}


	template <typename T>
	auto deque<T>::Block::at_spec(size_type loc) -> pair<Self*, pointer*> {
		if (loc >= size()) {
#ifndef STLite_NOEXCEPT
			if (succ->ishead())
				throw "in Block::at_spec(): argument loc too big";
#endif
			return succ->at_spec(loc - size());
		}
		return pair<Self*, pointer*>(this, __beg + loc);
	}

	template <typename T>
	auto deque<T>::Block::at_spec(size_type loc) const -> pair<const Self*, const pointer*> {
		if (loc >= size()) {
#ifndef STLite_NOEXCEPT
			if (succ->ishead())
				throw "in Block::at_spec(): argument loc too big";
#endif
			return succ->at_spec(loc - size());
		}
		return pair<const Self*, const pointer*>(this, __beg + loc);
	}

/* } */









	template <typename T>
	class deque<T>::iterator {
		friend class deque;
		friend class const_iterator;

		using Up		= deque;
		using Self		= iterator;
		using loc_type	= pair<Up::Block*, pointer*>;

		size_type index;
		loc_type loc;
		Up *par;

		auto eval_invalid() const -> bool { return par == nullptr or index >= par->size(); }
		auto exist_invalid() const -> bool { return par == nullptr or index > par->size(); }
		auto set_loc() -> void {
			if (par == nullptr) return;
			if (index >= par->size()) loc = par->end().loc;
			else loc = par->__data->succ->at_spec(index);
		}

		iterator(size_type __index, Up *__par)
			: index(__index), par(__par) { set_loc(); }
		iterator(size_type __index, loc_type __loc, Up *__par)
			: index(__index), loc(__loc), par(__par) { }

	public:
		iterator(): index(-1), loc(nullptr, nullptr), par(nullptr) { }
		iterator(Self &&) = default;
		iterator(const Self &) = default;

		auto operator = (const Self &) -> Self& = default;

		/**
		 * return a new iterator which pointer n-next elements
		 *   if there are not enough elements, iterator becomes invalid
		 * as well as operator-
		 */
		auto operator + (difference_type diff) const -> Self;
		auto operator - (difference_type diff) const -> Self;
		auto operator += (difference_type diff) -> Self& { return *this = *this + diff; }
		auto operator -= (difference_type diff) -> Self& { return *this = *this - diff; }

		// return the distance between two iterator,
		// if these two iterators points to different vectors, throw invaild_iterator.
		auto operator - (const Self &rhs) const -> difference_type {
			if (par != rhs.par) throw invalid_iterator();
			return static_cast<difference_type>(index - rhs.index);
		}

		auto operator ++ (int) -> Self { Self tmp = *this; *this = *this + 1; return tmp; }
		auto operator -- (int) -> Self { Self tmp = *this; *this = *this - 1; return tmp; }

		auto operator ++ () -> Self& { return *this = *this + 1; }
		auto operator -- () -> Self& { return *this = *this - 1; }

		auto operator * () const -> reference;
		auto operator -> () const -> pointer;

		auto operator == (const Self &rhs) const -> bool { return par == rhs.par and index == rhs.index; }
		auto operator == (const const_iterator &rhs) const -> bool { return par == rhs.par and index == rhs.index; }

		auto operator != (const Self &rhs) const -> bool { return par != rhs.par or index != rhs.index; }
		auto operator != (const const_iterator &rhs) const -> bool { return par != rhs.par or index != rhs.index; }
	};

/* impl deque<T>::iterator { */

	template <typename T>
	auto deque<T>::iterator::operator + (difference_type diff) const -> Self {
		if (diff < 0) return *this - (-diff);
		if (exist_invalid()) throw invalid_iterator();
		if (index + diff >= par->size())
			return par->end();
		Self dst = *this; dst.index += diff;
		for ( ; ; ) {
			difference_type rest = dst.loc.first->end() - dst.loc.second;
			if (diff < rest) return dst.loc.second += diff, dst;
			dst.loc.first = dst.loc.first->succ;
			dst.loc.second = dst.loc.first->begin();
			diff -= rest;
		}
	}

	template <typename T>
	auto deque<T>::iterator::operator - (difference_type diff) const -> Self {
		if (diff < 0) return *this + (-diff);
		if (exist_invalid()) throw invalid_iterator();
		if (index - diff < 0 or index - diff >= par->size())
			return par->end();
		Self dst = *this; dst.index -= diff;
		for ( ; ; ) {
			difference_type rest = dst.loc.second - dst.loc.first->begin();
			if (diff <= rest) return dst.loc.second -= diff, dst;
			dst.loc.first = dst.loc.first->prev;
			dst.loc.second = dst.loc.first->end();
			diff -= rest;
		}
	}

	template <typename T>
	auto deque<T>::iterator::operator * () const -> reference {
		if (eval_invalid()) throw invalid_iterator();
		return **loc.second;
	}

	template <typename T>
	auto deque<T>::iterator::operator -> () const -> pointer {
		if (eval_invalid()) throw invalid_iterator();
		return *loc.second;
	}

/* } */





	template <typename T>
	class deque<T>::const_iterator {
		friend class deque;
		friend class iterator;

		using Up		= deque;
		using Self		= const_iterator;
		using loc_type	= pair<const Up::Block*, const pointer*>;

		size_type index;
		loc_type loc;
		const Up *par;

		auto eval_invalid() const -> bool { return par == nullptr or index >= par->size(); }
		auto exist_invalid() const -> bool { return par == nullptr or index > par->size(); }
		auto set_loc() -> void {
			if (par == nullptr) return;
			if (index >= par->size()) loc = par->cend().loc;
			else loc = par->__data->succ->at_spec(index);
		}

		const_iterator(size_type __index, const Up *__par)
			: index(__index), par(__par) { set_loc(); }
		const_iterator(size_type __index, loc_type __loc, const Up *__par)
			: index(__index), loc(__loc), par(__par) { }

	public:
		const_iterator(): index(-1), loc(nullptr, nullptr), par(nullptr) { }
		const_iterator(Self &&) = default;
		const_iterator(const Self &) = default;
		const_iterator(const iterator &other): index(other.index), loc(loc), par(other.par) { }

		auto operator = (const Self &) -> Self& = default;

		/**
		 * return a new iterator which pointer n-next elements
		 *   if there are not enough elements, iterator becomes invalid
		 * as well as operator-
		 */
		auto operator + (difference_type diff) const -> Self;
		auto operator - (difference_type diff) const -> Self;
		auto operator += (difference_type diff) -> Self& { return *this = *this + diff; }
		auto operator -= (difference_type diff) -> Self& { return *this = *this - diff; }

		// return the distance between two iterator,
		// if these two iterators points to different vectors, throw invaild_iterator.
		auto operator - (const Self &rhs) const -> difference_type {
			if (par != rhs.par) throw invalid_iterator();
			return static_cast<difference_type>(index - rhs.index);
		}

		auto operator ++ (int) -> Self { Self tmp = *this; *this = *this + 1; return tmp; }
		auto operator -- (int) -> Self { Self tmp = *this; *this = *this - 1; return tmp; }

		auto operator ++ () -> Self& { return *this = *this + 1; }
		auto operator -- () -> Self& { return *this = *this - 1; }

		auto operator * () const -> const_reference;
		auto operator -> () const -> const_pointer;

		auto operator == (const Self &rhs) const -> bool { return par == rhs.par and index == rhs.index; }
		auto operator == (const iterator &rhs) const -> bool { return par == rhs.par and index == rhs.index; }

		auto operator != (const Self &rhs) const -> bool { return par != rhs.par or index != rhs.index; }
		auto operator != (const iterator &rhs) const -> bool { return par != rhs.par or index != rhs.index; }
	};

/* impl deque<T>::const_iterator { */

	template <typename T>
	auto deque<T>::const_iterator::operator + (difference_type diff) const -> Self {
		if (diff < 0) return *this - (-diff);
		if (exist_invalid()) throw invalid_iterator();
		if (index + diff >= par->size())
			return par->cend();
		Self dst = *this; dst.index += diff;
		for ( ; ; ) {
			difference_type rest = dst.loc.first->cend() - dst.loc.second;
			if (diff < rest) return dst.loc.second += diff, dst;
			dst.loc.first = dst.loc.first->succ;
			dst.loc.second = dst.loc.first->cbegin();
			diff -= rest;
		}
	}

	template <typename T>
	auto deque<T>::const_iterator::operator - (difference_type diff) const -> Self {
		if (diff < 0) return *this + (-diff);
		if (exist_invalid()) throw invalid_iterator();
		if (index - diff < 0 or index - diff >= par->size())
			return par->cend();
		Self dst = *this; dst.index -= diff;
		for ( ; ; ) {
			difference_type rest = dst.loc.second - dst.loc.first->cbegin();
			if (diff <= rest) return dst.loc.second -= diff, dst;
			dst.loc.first = dst.loc.first->prev;
			dst.loc.second = dst.loc.first->cend();
			diff -= rest;
		}
	}

	template <typename T>
	auto deque<T>::const_iterator::operator * () const -> const_reference {
		if (eval_invalid()) throw invalid_iterator();
		return **loc.second;
	}

	template <typename T>
	auto deque<T>::const_iterator::operator -> () const -> const_pointer {
		if (eval_invalid()) throw invalid_iterator();
		return *loc.second;
	}

/* } */










/* impl deque<T> { */

	template <typename T>
	deque<T>::deque(): __size(0),
		__data(new Block(typename Block::noinit_tag())) {
			__data->link(new Block);
		}

	template <typename T>
	deque<T>::deque(Self &&other): __size(other.__size),
		__data(new Block(typename Block::noinit_tag())) {
			__data->prev = other.__data->prev;
			__data->succ = other.__data->succ;
			other.__data->prev = other.__data->succ = other.__data;
		}

	template <typename T>
	deque<T>::deque(const Self &other):
		__size(other.__size), __data(other.__data->clone()) { }

	template <typename T>
	auto deque<T>::operator = (Self &&rhs) -> Self& {
		if (this == std::addressof(rhs)) return *this;
		clear(); delete __data; __size = rhs.__size;
		__data->prev = rhs.__data->prev;
		__data->succ = rhs.__data->succ;
		rhs.__data->prev = rhs.__data->succ = rhs.__data;
		return *this;
	}

	template <typename T>
	auto deque<T>::operator = (const Self &rhs) -> Self& {
		if (this == std::addressof(rhs)) return *this;
		clear(); delete __data; __size = rhs.__size;
		__data = rhs.__data->clone();
		return *this;
	}


	template <typename T>
	auto deque<T>::at(const size_type &loc) -> reference {
		if (loc >= size()) throw index_out_of_bound();
#ifndef STLite_NOEXCEPT
		try {
			return __data->at(loc);
		} catch (const char *msg) {
			puts("\nexception caught!");
			puts(msg);
			throw index_out_of_bound();
		}
#else
			return __data->at(loc);
#endif
	}

	template <typename T>
	auto deque<T>::at(const size_type &loc) const -> const_reference {
		if (loc >= size()) throw index_out_of_bound();
#ifndef STLite_NOEXCEPT
		try {
			return __data->at(loc);
		} catch (const char *msg) {
			puts("\nexception caught!");
			puts(msg);
			throw index_out_of_bound();
		}
#else
			return __data->at(loc);
#endif
	}


	template <typename T>
	auto deque<T>::front() -> reference {
		if (empty()) throw container_is_empty();
		return __data->succ->front();
	}

	template <typename T>
	auto deque<T>::front() const -> const_reference {
		if (empty()) throw container_is_empty();
		return __data->succ->front();
	}

	template <typename T>
	auto deque<T>::back() -> reference {
		if (empty()) throw container_is_empty();
		return __data->prev->back();
	}

	template <typename T>
	auto deque<T>::back() const -> const_reference {
		if (empty()) throw container_is_empty();
		return __data->prev->back();
	}


	template <typename T>
	auto deque<T>::clear() -> void {
		for (; __data->succ != __data; )
			delete __data->cut();
		__size = 0;
		__data->link(new Block);
	}

	template <typename T>
	auto deque<T>::insert(iterator it, const value_type &value) -> iterator {
		if (it.par != this or it.index > size())
			throw invalid_iterator();
		it.loc.first->insert(it.loc.second - it.loc.first->begin(), value);
		++__size;
		return iterator(it.index, this);
	}

	template <typename T>
	auto deque<T>::erase(iterator it) -> iterator {
		if (empty()) throw container_is_empty();
		if (it.par != this or it.index >= size())
			throw invalid_iterator();
		it.loc.first->erase(it.loc.second - it.loc.first->begin());
		--__size;
		return iterator(it.index, this);
	}

	template <typename T>
	auto deque<T>::pop_front() -> void {
		if (empty()) throw container_is_empty();
		__data->succ->pop_front(); --__size;
	}

	template <typename T>
	auto deque<T>::pop_back() -> void {
		if (empty()) throw container_is_empty();
		__data->prev->pop_back(); --__size;
	}

/* } */


}

#endif
