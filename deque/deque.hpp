#ifndef SJTU_DEQUE_HPP
#define SJTU_DEQUE_HPP

#include "exceptions.hpp"

#include <cstddef>

namespace sjtu {

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
	auto operator[](const size_t &pos) -> reference { return at(pos); }
	auto operator[](const size_t &pos) const -> const_reference { return at(pos); }

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

	auto begin() -> iterator { return iterator(0, this); }
	auto cbegin() const -> const_iterator { return const_iterator(0, this); }

	auto end() -> iterator { return iterator(size(), this); }
	auto cend() const -> const_iterator { return const_iterator(size(), this); }

	auto empty() const -> bool { return __size == 0; }
	auto size() const -> size_type { return __size; }

	auto clear() -> void;

	/**
	 * inserts elements at the specified locat on in the container.
	 * inserts value before pos
	 * returns an iterator pointing to the inserted value
	 *     throw if the iterator is invalid or it point to a wrong place.
	 */
	auto insert(iterator, const value_type &) -> iterator;

	/**
	 * removes specified element at pos.
	 * removes the element at pos.
	 * returns an iterator pointing to the following element, if pos pointing to the last element, end() will be returned.
	 * throw if the container is empty, the iterator is invalid or it points to a wrong place.
	 */
	auto erase(iterator pos) -> iterator;

	/**
	 * inserts an element to the beginning.
	 */
	auto push_front(const value_type &value) -> void { __data->insert(0, value); }

	/**
	 * adds an element to the end
	 */
	auto push_back(const value_type &value) -> void { __data->insert(size(), value); }

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
		pointer __data, __beg, __end;

		Block();
		Block(noinit_tag);
		Block(const_pointer, size_type);
		Block(const Self &);
		~Block();

		auto clone() const -> Self*;
		auto data() const -> pointer { return __beg; }
		auto size() const -> size_type { return __end - __beg; }
		auto cap_back() const -> size_type { return __data + BLOCK_SIZE - __end; }

		auto link(Self *) -> Self*;
		auto cut() -> Self*;

		auto append_unchecked(const_pointer, size_type) -> void;
		auto append(const_pointer, size_type) -> void;
		auto move_data_in_block(pointer) -> void;
		auto reserve(size_type) -> void;

		auto adjust() -> void;

		auto merge() -> Self*;
		auto split(size_type) -> Self*;

		auto push_front(const value_type &) -> void;
		auto push_back(const value_type &) -> void;

		auto pop_front() -> void;
		auto pop_back() -> void;

		auto insert(size_type, const value_type &) -> void;
		auto erase(size_type) -> void;

		auto at(size_type) -> reference;
		auto at(size_type) const -> const_reference;
	};

/* impl deque<T>::Block { */

	template <typename T>
	deque<T>::Block::Block()
		: prev(nullptr), succ(nullptr),
			__data(static_cast<pointer>(std::malloc(sizeof(value_type) * BLOCK_SIZE))),
				__beg(__data + HALF_BLOCK_SIZE), __end(__beg) {}

	template <typename T>
	deque<T>::Block::Block(noinit_tag)
		: prev(nullptr), succ(nullptr), __data(nullptr), __beg(nullptr), __end(nullptr) {}

	template <typename T>
	deque<T>::Block::Block(const_pointer src, size_type n): Block() {
		__beg = __end = __data + HALF_BLOCK_SIZE - n / 2;
		append_unchecked(src, n);
	}

	template <typename T>
	deque<T>::Block::Block(const Self &other)
		: Block(other.data(), other.size()) { }

	template <typename T>
	deque<T>::Block::~Block() {
		for (; __end != __beg; )
			(--__end)->~value_type();
		free(__data);
	}


	template <typename T>
	auto deque<T>::Block::clone() const -> Self* {
		Self *target = new Block(*this);
		if (succ != nullptr) target->link(succ->clone());
		return target;
	}

	template <typename T>
	auto deque<T>::Block::link(Self *target) -> Self* {
		if (target == nullptr)
#ifdef DEBUG
			throw "call Block::link() with a null pointer: this will cause original successor lost!";
#endif
			return nullptr;
		Self *u = succ; succ = target; target->succ = u;
		target->prev = this; if (u != nullptr) u->prev = target;
		return target;
	}

	template <typename T>
	auto deque<T>::Block::cut() -> Self* {
		if (succ == nullptr)
#ifdef DEBUG
			throw "call Block::cut() on a block without succ block: this will cut nothing";
#endif
			return nullptr;
		Self *target = succ, *u = target->succ;
		succ = u; if (u != nullptr) u->prev = this;
		target->prev = target->succ = nullptr;
		return target;
	}


	template <typename T>
	auto deque<T>::Block::append_unchecked(const_pointer src, size_type n) -> void {
		for (; n > 0; --n)
			new(__end++) value_type(*src++);
	}

	template <typename T>
	auto deque<T>::Block::append(const_pointer src, size_type n) -> void {
		for (; n > 0; ) {
			auto step = std::min(n, cap_back());
			append_unchecked(src, step);
			adjust();
			src += step; n -= step;
		}
	}

	template <typename T>
	auto deque<T>::Block::move_data_in_block(pointer __dst) -> void {
		if (__dst < __data or __data + BLOCK_SIZE <= __dst)
#ifdef DEBUG
			throw "call Block::move_data_in_block(): __dst not in block";
#endif
			return;
		if (__beg < __dst) {
			auto o_beg = __beg, o_end = __end;
			__beg = __end = __dst + size();
			for (; o_beg != o_end; )
				new(--__beg) value_type(*--o_end);
		} else if (__beg > __dst) {
			auto o_beg = __beg, o_end = __end;
			__beg = __end = __dst;
			for (; o_beg != o_end; )
				new(__end++) value_type(*o_beg++);
		}
	}

	template <typename T>
	auto deque<T>::Block::reserve(size_type __size) -> void {
		if (__size <= size()) return;
		if (__size > BLOCK_SIZE)
#ifdef DEBUG
			throw "call Block::reserve(): reserve size too large";
#endif
			return;
		move_data_in_block(__data + HALF_BLOCK_SIZE - __size / 2);
	}

	template <typename T>
	auto deque<T>::Block::adjust() -> void {
		if (succ != nullptr and size() + succ->size() < CONTENT_LIMIT) {
			reserve(size() + succ->size());
			merge();
		}
		if (size() >= CONTENT_LIMIT * 2)
			split(size() / 2);
		if (__beg == __data or __end == __data + BLOCK_SIZE)
			move_data_in_block(__data + HALF_BLOCK_SIZE - size() / 2);
	}


	template <typename T>
	auto deque<T>::Block::merge() -> Self* {
		if (succ == nullptr)
#ifdef DEBUG
			throw "call Block::merge() on a block without succ block";
#endif
			return this;
		Block *target = cut();
		append(target->data(), target->size());
		delete target;
		return this;
	}

	template <typename T>
	auto deque<T>::Block::split(size_type pos) -> Self* {
		if (pos > size())
#ifdef DEBUG
			throw "call Block::split() with argument pos too big";
#endif
			return this;
		link(new Block(__beg + pos, size() - pos));
		__end = __beg + pos;
		return succ;
	}


	template <typename T>
	auto deque<T>::Block::push_front(const value_type &value) -> void {
		new(--__beg) value_type(value);
		adjust();
	}

	template <typename T>
	auto deque<T>::Block::push_back(const value_type &value) -> void {
		new(__end++) value_type(value);
		adjust();
	}

	template <typename T>
	auto deque<T>::Block::pop_front() -> void {
		if (size() == 0)
#ifdef DEBUG
			throw "call Block::pop_front() on an empty block";
#endif
			return;
		(__beg++)->~value_type();
		adjust();
	}

	template <typename T>
	auto deque<T>::Block::pop_back() -> void {
		if (size() == 0)
#ifdef DEBUG
			throw "call Block::pop_back() on an empty block";
#endif
			return;
		(--__end)->~value_type();
		adjust();
	}


	template <typename T>
	auto deque<T>::Block::insert(size_type pos, const value_type &value) -> void {
		if (pos > size()) {
			if (succ == nullptr)
#ifdef DEBUG
				throw "in Block::insert(): argument pos too big";
#endif
				return;
			return succ->insert(pos - size(), value);
		}
		if (prev == nullptr and pos == 0)
			return link(new Block)->insert(0, value);

		if (pos * 2 < size()) {
			for (auto p = __beg; p != __beg + pos; ++p)
				new(p) value_type(*(p + 1));
			--__beg;
		} else {
			for (auto p = __end; p != __beg + pos; --p)
				new(p) value_type(*(p - 1));
			__end++;
		}
		new(__beg + pos) value_type(value);

		adjust();
	}

	template <typename T>
	auto deque<T>::Block::erase(size_type pos) -> void {
		if (pos >= size()) {
			if (succ == nullptr)
#ifdef DEBUG
				throw "in Block::erase(): argument pos too big";
#endif
				return;
			return succ->erase(pos - size());
		}

		if (pos * 2 < size()) {
			for (auto p = __beg + pos; p != __beg; --p)
				new(p) value_type(*(p - 1));
			++__beg;
		} else {
			--__end;
			for (auto p = __beg + pos; p != __end; ++p)
				new(p) value_type(*(p + 1));
		}

		adjust();
	}

	template <typename T>
	auto deque<T>::Block::at(size_type pos) -> reference {
		if (pos >= size()) {
			if (succ == nullptr)
#ifdef DEBUG
				throw "in Block::at(): argument pos too big";
#endif
				return __beg[size() - 1];
			return succ->at(pos - size());
		}
		return __beg[pos];
	}

	template <typename T>
	auto deque<T>::Block::at(size_type pos) const -> const_reference {
		if (pos >= size()) {
			if (succ == nullptr)
#ifdef DEBUG
				throw "in Block::at(): argument pos too big";
#endif
				return __beg[size() - 1];
			return succ->at(pos - size());
		}
		return __beg[pos];
	}

/* } */

	template <typename T>
	class deque<T>::iterator {
		friend class deque;
		friend class const_iterator;

		using Up	= Self;
		using Self	= iterator;

		size_type pos;
		Up *par;

		iterator(size_type __pos, Up *__par)
			: pos(__pos), par(__par) {}

	public:
		iterator(): pos(-1), par(nullptr) {}
		iterator(Self &&) = default;
		iterator(const Self &) = default;

		auto operator = (const Self &) -> Self& = default;

		/**
		 * return a new iterator which pointer n-next elements
		 *   if there are not enough elements, iterator becomes invalid
		 * as well as operator-
		 */
		auto operator + (difference_type diff) const -> Self { return Self(pos + diff, par); }
		auto operator - (difference_type diff) const -> Self { return Self(pos - diff, par); }
		auto operator += (difference_type diff) -> Self& { return pos += diff, *this; }
		auto operator -= (difference_type diff) -> Self& { return pos -= diff, *this; }

		// return the distance between two iterator,
		// if these two iterators points to different vectors, throw invaild_iterator.
		auto operator - (const Self &rhs) const -> difference_type {
			if (par != rhs.par) throw invalid_iterator();
			return static_cast<difference_type>(pos - rhs.pos);
		}

		auto operator ++ (int) -> Self { return Self(pos++, par); }
		auto operator -- (int) -> Self { return Self(pos--, par); }

		auto operator ++ () -> Self& { return ++pos, *this; }
		auto operator -- () -> Self& { return --pos, *this; }

		auto operator * () const -> reference;
		auto operator -> () const noexcept -> pointer;

		auto operator == (const Self &rhs) const -> bool { return par == rhs.par and pos == rhs.pos; }
		auto operator == (const const_iterator &rhs) const -> bool { return par == rhs.par and pos == rhs.pos; }

		auto operator != (const Self &rhs) const -> bool { return par != rhs.par or pos != rhs.pos; }
		auto operator != (const const_iterator &rhs) const -> bool { return par != rhs.par or pos != rhs.pos; }
	};

/* impl deque<T>::iterator { */

	template <typename T>
	auto deque<T>::iterator::operator * () const -> reference {
		if (par == nullptr or pos >= par->size())
			throw invalid_iterator();
		return par->__data->at(pos);
	}

	template <typename T>
	auto deque<T>::iterator::operator -> () const noexcept -> pointer {
		if (par == nullptr or pos >= par->size())
			throw invalid_iterator();
		return std::addressof(par->__data->at(pos));
	}

/* } */

	template <typename T>
	class deque<T>::const_iterator {
		friend class deque;
		friend class iterator;

		using Up	= Self;
		using Self	= const_iterator;

		size_type pos;
		Up *par;

		const_iterator(size_type __pos, Up *__par)
			: pos(__pos), par(__par) {}

	public:
		const_iterator(): pos(-1), par(nullptr) {}
		const_iterator(Self &&) = default;
		const_iterator(const Self &) = default;
		const_iterator(const iterator &other): pos(other.pos), par(other.par) {}

		auto operator = (const Self &) -> Self& = default;

		/**
		 * return a new iterator which pointer n-next elements
		 *   if there are not enough elements, iterator becomes invalid
		 * as well as operator-
		 */
		auto operator + (difference_type diff) const -> Self { return Self(pos + diff, par); }
		auto operator - (difference_type diff) const -> Self { return Self(pos - diff, par); }
		auto operator += (difference_type diff) -> Self& { return pos += diff, *this; }
		auto operator -= (difference_type diff) -> Self& { return pos -= diff, *this; }

		// return th distance between two iterator,
		// if these two iterators points to different vectors, throw invaild_iterator.
		auto operator - (const Self &rhs) const -> difference_type {
			if (par != rhs.par) throw invalid_iterator();
			return static_cast<difference_type>(pos - rhs.pos);
		}

		auto operator ++ (int) -> Self { return Self(pos++, par); }
		auto operator -- (int) -> Self { return Self(pos--, par); }

		auto operator ++ () -> Self& { return ++pos, *this; }
		auto operator -- () -> Self& { return --pos, *this; }

		auto operator * () const -> const_reference;
		auto operator -> () const noexcept -> const_pointer;

		auto operator == (const Self &rhs) const -> bool { return par == rhs.par and pos == rhs.pos; }
		auto operator == (const iterator &rhs) const -> bool { return par == rhs.par and pos == rhs.pos; }

		auto operator != (const Self &rhs) const -> bool { return par != rhs.par or pos != rhs.pos; }
		auto operator != (const iterator &rhs) const -> bool { return par != rhs.par or pos != rhs.pos; }
	};

/* impl deque<T>::const_iterator { */

	template <typename T>
	auto deque<T>::const_iterator::operator * () const -> const_reference {
		if (par == nullptr or pos >= par->size())
			throw invalid_iterator();
		return par->__data->at(pos);
	}

	template <typename T>
	auto deque<T>::const_iterator::operator -> () const noexcept -> const_pointer {
		if (par == nullptr or pos >= par->size())
			throw invalid_iterator();
		return std::addressof(par->__data->at(pos));
	}

/* } */

/* impl deque<T> { */

	template <typename T>
	deque<T>::deque(): __size(0),
		__data(new Block(typename Block::noinit_tag())) {}

	template <typename T>
	deque<T>::deque(Self &&other): __size(other.__size),
		__data(new Block(typename Block::noinit_tag())) {
			__data->succ = other.__data->succ;
			other.__data->succ = nullptr;
		}

	template <typename T>
	deque<T>::deque(const Self &other): __size(other.__size),
		__data(new Block(typename Block::noinit_tag())) {
			if (other.size() != 0)
				__data->succ = other.__data->succ->clone();
		}

	template <typename T>
	auto deque<T>::operator = (Self &&rhs) -> Self& {
		if (this == std::addressof(rhs)) return *this;
		clear();
		__data->succ = rhs.__data->succ;
		rhs.__data->succ = nullptr;
		return *this;
	}

	template <typename T>
	auto deque<T>::operator=(const Self &rhs) -> Self& {
		if (this == std::addressof(rhs)) return *this;
		clear();
		if (rhs.size() != 0) __data->succ = rhs.__data->succ->clone();
		return *this;
	}


	template <typename T>
	auto deque<T>::at(const size_type &pos) -> reference {
		try {
			return __data->at(pos);
		} catch (const char *msg) {
			puts("exception caught!");
			puts(msg);
			throw index_out_of_bound{};
		}
	}

	template <typename T>
	auto deque<T>::at(const size_type &pos) const -> const_reference {
		try {
			return __data->at(pos);
		} catch (const char *msg) {
			puts("exception caught!");
			puts(msg);
			throw index_out_of_bound{};
		}
	}


	template <typename T>
	auto deque<T>::front() -> reference {
		if (size() == 0) throw container_is_empty{};
		return at(0);
	}

	template <typename T>
	auto deque<T>::front() const -> const_reference {
		if (size() == 0) throw container_is_empty{};
		return at(0);
	}

	template <typename T>
	auto deque<T>::back() -> reference {
		if (size() == 0) throw container_is_empty{};
		return at(size() - 1);
	}

	template <typename T>
	auto deque<T>::back() const -> const_reference {
		if (size() == 0) throw container_is_empty{};
		return at(size() - 1);
	}


	template <typename T>
	auto deque<T>::clear() -> void {
		for (; __data->succ != nullptr; )
			delete __data->cut();
	}

	template <typename T>
	auto deque<T>::insert(iterator pos, const value_type &value) -> iterator {
		if (pos.par != this or pos.pos > size())
			throw invalid_iterator();
		__data->insert(pos.pos, value);
		return pos;
	}

	template <typename T>
	auto deque<T>::erase(iterator pos) -> iterator {
		if (empty()) throw container_is_empty();
		if (pos.par != this or pos.pos >= size())
			throw invalid_iterator();
		__data->erase(pos.pos);
		return pos;
	}

	template <typename T>
	auto deque<T>::pop_front() -> void {
		if (size() == 0) throw container_is_empty();
		__data->erase(0);
	}

	template <typename T>
	auto deque<T>::pop_back() -> void {
		if (size() == 0) throw container_is_empty();
		__data->erase(size() - 1);
	}

/* } */


}

#endif
