/**
 * implement a container like std::map
 */
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include "exceptions.hpp"

namespace sjtu {

// #define STlite_NOEXCEPT

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


template <class Key, class T, class Compare = std::less<Key>>
class map {
	using Self = map;
public:
	using key_typ			= Key;
	using mapped_type		= T;
	using key_compare		= Compare;
	using value_type		= pair<const Key, T>;
	using reference			= value_type&;
	using const_reference	= const value_type&;
	using pointer			= value_type*;
	using const_pointer		= const value_type*;
	using size_type			= size_t;
	using difference_type	= std::ptrdiff_t;

	/**
	 * see BidirectionalIterator at CppReference for help.
	 *
	 * if there is anything wrong throw invalid_iterator.
	 *     like it = map.begin(); --it;
	 *       or it = map.end(); ++end();
	 */
	class iterator;
	class const_iterator;

	map();
	map(const Self &other);
	auto operator = (const Self &other) -> Self&;
	~map();
	/**
	 * TODO
	 * access specified element with bounds checking
	 * Returns a reference to the mapped value of the element with key equivalent to key.
	 * If no such element exists, an exception of type `index_out_of_bound'
	 */
	auto at(const Key &key) -> mapped_type&;
	auto at(const Key &key) const -> const mapped_type&;
	/**
	 * TODO
	 * access specified element
	 * Returns a reference to the value that is mapped to a key equivalent to key,
	 *   performing an insertion if such key does not already exist.
	 */
	auto operator[](const Key &key) -> mapped_type&;
	/**
	 * behave like at() throw index_out_of_bound if such key does not exist.
	 */
	auto operator[](const Key &key) const -> const mapped_type&;
	/**
	 * return a iterator to the beginning
	 */
	auto begin() -> iterator;
	auto cbegin() const -> const_iterator;
	/**
	 * return a iterator to the end
	 * in fact, it returns past-the-end.
	 */
	auto end() -> iterator;
	auto cend() const -> const_iterator;
	/**
	 * checks whether the container is empty
	 * return true if empty, otherwise false.
	 */
	auto empty() const -> bool;
	/**
	 * returns the number of elements.
	 */
	auto size() const -> size_type;
	/**
	 * clears the contents
	 */
	auto clear() -> void;
	/**
	 * insert an element.
	 * return a pair, the first of the pair is
	 *   the iterator to the new element (or the element that prevented the insertion),
	 *   the second one is true if insert successfully, or false.
	 */
	auto insert(const value_type &value) -> pair<iterator, bool>;
	/**
	 * erase the element at pos.
	 *
	 * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
	 */
	auto erase(iterator pos) -> void;
	/**
	 * Returns the number of elements with key
	 *   that compares equivalent to the specified argument,
	 *   which is either 1 or 0
	 *     since this container does not allow duplicates.
	 * The default method of check the equivalence is !(a < b || b > a)
	 */
	auto count(const Key &key) const -> size_type;
	/**
	 * Finds an element with key equivalent to key.
	 * key value of the element to search for.
	 * Iterator to an element with key equivalent to key.
	 *   If no such element is found, past-the-end (see end()) iterator is returned.
	 */
	auto find(const Key &key) -> iterator;
	auto find(const Key &key) const -> const_iterator;
};



	template <class Key, class T, class Compare>
	class map<Key, T, Compare>::iterator {
		using Self = iterator;
		/**
		 * TODO add data members
		 *   just add whatever you want.
		 */
	public:
		iterator() {
			// TODO
		}
		iterator(const Self &other) {
			// TODO
		}
		/**
		 * TODO iter++
		 */
		auto operator++(int) -> Self;
		/**
		 * TODO ++iter
		 */
		auto operator++() -> Self&;
		/**
		 * TODO iter--
		 */
		auto operator--(int) -> Self;
		/**
		 * TODO --iter
		 */
		auto operator--() -> Self&;
		/**
		 * a operator to check whether two iterators are same (pointing to the same memory).
		 */
		auto operator*() const -> reference;
		auto operator==(const iterator &rhs) const -> bool;
		auto operator==(const const_iterator &rhs) const -> bool;
		/**
		 * some other operator for iterator.
		 */
		auto operator!=(const iterator &rhs) const -> bool;
		auto operator!=(const const_iterator &rhs) const -> bool;

		/**
		 * for the support of it->first.
		 * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
		 */
		auto operator->() const noexcept -> pointer;
	};

	template <class Key, class T, class Compare>
	class map<Key, T, Compare>::const_iterator {
			using Self = const_iterator;
		private:
			// data members.
		public:
			const_iterator() {
				// TODO
			}
			const_iterator(const Self &other) {
				// TODO
			}
			const_iterator(const iterator &other) {
				// TODO
			}
			// And other methods in iterator.
			// And other methods in iterator.
			// And other methods in iterator.
	};

}

#endif
