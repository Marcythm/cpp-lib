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
	// struct Wrap;
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
		auto cap_back() const -> size_type { return __data + BLOCK_SIZE - __end; }
		auto clear() -> void { __beg = __end = __data + HALF_BLOCK_SIZE; }
		auto ishead() const -> bool { return __data == nullptr; }
		auto empty() const -> bool { return __beg == __end; }

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
		if (__dst < __data or __data + BLOCK_SIZE <= __dst)
			throw "call Block::move_data_in_block(): __dst not in block";
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
		if (__size > BLOCK_SIZE)
			throw "call Block::reserve(): reserve size too large";
		move_data_in_block(__data + HALF_BLOCK_SIZE - __size / 2);
	}


	template <typename T>
	auto deque<T>::Block::link(Self *target) -> Self* {
		if (target == nullptr)
			throw "call Block::link() with a null pointer: this will cause original successor lost!";
		Self *s = this, *u = target, *v = target->prev, *t = succ;
		s->succ = u; u->prev = s;
		v->succ = t; t->prev = v;
		return target;
	}

	template <typename T>
	auto deque<T>::Block::cut() -> Self* {
		if (succ->ishead() or (prev == this and succ == this))
			throw "call Block::cut() on a block without succ block: this will cut itself";
		Self *target = succ, *u = target->succ;
		succ = u; u->prev = this;
		target->prev = target->succ = target;
		return target;
	}

	template <typename T>
	auto deque<T>::Block::merge() -> Self* {
		if (succ->ishead())
			throw "call Block::merge() on a block without succ block";
		Self *target = cut();
		append(target->data(), target->size(), true);
		target->clear();
		delete target;
		return this;
	}

	template <typename T>
	auto deque<T>::Block::split(size_type pos) -> Self* {
		if (pos > size())
			throw "call Block::split() with argument pos too big";
		link(new Block(__beg + pos, size() - pos));
		__end = __beg + pos;
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
		if (empty())
			throw "call Block::pop_front() on an empty block";
		delete *__beg++;
		adjust();
	}

	template <typename T>
	auto deque<T>::Block::pop_back() -> void {
		if (empty())
			throw "call Block::pop_back() on an empty block";
		delete *--__end;
		adjust();
	}


	template <typename T>
	auto deque<T>::Block::insert(size_type pos, const value_type &value) -> void {
		if (pos > size()) {
			if (succ->ishead())
				throw "in Block::insert(): argument pos too big";
			return succ->insert(pos - size(), value);
		}

		if (pos * 2 < size()) {
			for (auto p = __beg; p != __beg + pos; ++p)
				*(p - 1) = *p;
			--__beg;
		} else {
			for (auto p = __end; p != __beg + pos; --p)
				*p = *(p - 1);
			__end++;
		}
		__beg[pos] = new value_type(value);

		adjust();
	}

	template <typename T>
	auto deque<T>::Block::erase(size_type pos) -> void {
		if (pos >= size()) {
			if (succ->ishead())
				throw "in Block::erase(): argument pos too big";
			return succ->erase(pos - size());
		}

		delete __beg[pos];
		if (pos * 2 < size()) {
			for (auto p = __beg + pos; p != __beg; --p)
				*p = *(p - 1);
			__beg++;
		} else {
			--__end;
			for (auto p = __beg + pos; p != __end; ++p)
				*p = *(p + 1);
		}

		adjust();
	}

	template <typename T>
	auto deque<T>::Block::front() -> reference {
		if (empty()) throw "call Block::front() on an empty block";
		return *__beg[0];
	}

	template <typename T>
	auto deque<T>::Block::front() const -> const_reference {
		if (empty()) throw "call Block::front() on an empty block";
		return *__beg[0];
	}

	template <typename T>
	auto deque<T>::Block::back() -> reference {
		if (empty()) throw "call Block::back() on an empty block";
		return *__end[-1];
	}

	template <typename T>
	auto deque<T>::Block::back() const -> const_reference {
		if (empty()) throw "call Block::back() on an empty block";
		return *__end[-1];
	}


	template <typename T>
	auto deque<T>::Block::at(size_type pos) -> reference {
		if (pos >= size()) {
			if (succ->ishead())
				throw "in Block::at(): argument pos too big";
			return succ->at(pos - size());
		}
		return *__beg[pos];
	}

	template <typename T>
	auto deque<T>::Block::at(size_type pos) const -> const_reference {
		if (pos >= size()) {
			if (succ->ishead())
				throw "in Block::at(): argument pos too big";
			return succ->at(pos - size());
		}
		return *__beg[pos];
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
			: pos(__pos), par(__par) { }

	public:
		iterator(): pos(-1), par(nullptr) { }
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
		auto operator -> () const -> pointer;

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
	auto deque<T>::iterator::operator -> () const -> pointer {
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
		const Up *par;

		const_iterator(size_type __pos, const Up *__par)
			: pos(__pos), par(__par) { }

	public:
		const_iterator(): pos(-1), par(nullptr) { }
		const_iterator(Self &&) = default;
		const_iterator(const Self &) = default;
		const_iterator(const iterator &other): pos(other.pos), par(other.par) { }

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

		auto operator * () const -> const_reference;
		auto operator -> () const -> const_pointer;

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
	auto deque<T>::const_iterator::operator -> () const -> const_pointer {
		if (par == nullptr or pos >= par->size())
			throw invalid_iterator();
		return std::addressof(par->__data->at(pos));
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
	auto deque<T>::at(const size_type &pos) -> reference {
		if (pos >= size()) throw index_out_of_bound();
		try {
			return __data->at(pos);
		} catch (const char *msg) {
			puts("\nexception caught!");
			puts(msg);
			throw index_out_of_bound();
		}
	}

	template <typename T>
	auto deque<T>::at(const size_type &pos) const -> const_reference {
		if (pos >= size()) throw index_out_of_bound();
		try {
			return __data->at(pos);
		} catch (const char *msg) {
			puts("\nexception caught!");
			puts(msg);
			throw index_out_of_bound();
		}
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
	auto deque<T>::insert(iterator pos, const value_type &value) -> iterator {
		if (pos.par != this or pos.pos > size())
			throw invalid_iterator();
		__data->succ->insert(pos.pos, value); ++__size;
		return pos;
	}

	template <typename T>
	auto deque<T>::erase(iterator pos) -> iterator {
		if (empty()) throw container_is_empty();
		if (pos.par != this or pos.pos >= size())
			throw invalid_iterator();
		__data->succ->erase(pos.pos); --__size;
		return pos;
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
