#ifndef SJTU_VECTOR_HPP
#define SJTU_VECTOR_HPP

#include "exceptions.hpp"

#include <climits>
#include <cstddef>

namespace sjtu {
/**
 * a data container like std::vector
 * store data in a successive memory and support random access.
 */
template <typename T>
class vector {
    using Self = vector;
public:
    using value_type		= T;
    using reference			= value_type&;
    using const_reference	= const value_type&;
    using pointer			= value_type*;
    using const_pointer		= const value_type*;

    using size_type			= size_t;
    using difference_type	= std::ptrdiff_t;

    class iterator;
    class const_iterator;

private:
    pointer *__beg, *__end, *__cap;

    auto cap_full() const -> bool { return __end == __cap; }

public:
    vector();
    vector(Self &&);
    vector(const Self &);

    ~vector();

    auto operator = (Self &&) -> Self&;
    auto operator = (const Self &) -> Self&;

    auto empty() const -> bool { return __beg == __end; }
    auto size() const -> size_type { return __end - __beg; }
    auto capacity() const -> size_type { return __cap - __beg; }
    auto clear() -> void;

    auto reserve(size_type) -> void;

    auto at(size_type) -> reference;
    auto at(size_type) const -> const_reference;

    auto operator[](size_type loc) -> reference { return at(loc); }
    auto operator[](size_type loc) const -> const_reference { return at(loc); }

    auto front() -> reference;
    auto front() const -> const_reference;
    auto back() -> reference;
    auto back() const -> const_reference;

    auto begin() -> iterator { return iterator(__beg, this); }
    auto begin() const -> iterator { return iterator(__beg, this); }
    auto cbegin() const -> const_iterator { return const_iterator(__beg, this); }

    auto end() -> iterator { return iterator(__end, this); }
    auto end() const -> iterator { return iterator(__end, this); }
    auto cend() const -> const_iterator { return const_iterator(__end, this); }

    auto insert(iterator, const value_type &) -> iterator;
    auto insert(size_type, const value_type &) -> iterator;
    auto erase(iterator) -> iterator;
    auto erase(size_type) -> iterator;

    auto push_back(const value_type &) -> void;
    auto pop_back() -> void;
};



    template <typename T>
    class vector<T>::iterator {
        using Self	= iterator;
        using Up	= vector<T>;

        friend class const_iterator;
        friend class vector<T>;

        pointer *loc;
        Up *par;
        iterator(pointer *__loc, Up *__par): loc(__loc), par(__par) { }

    public:
        iterator(): loc(nullptr), par(nullptr) { }

        auto operator + (difference_type diff) const -> Self { return iterator(loc + diff, par); }
        auto operator - (difference_type diff) const -> Self { return iterator(loc - diff, par); }
        // return the distance between two iterators,
        // if these two iterators point to different vectors, throw invaild_iterator.
        auto operator - (const Self &rhs) const -> difference_type {
            if (par != rhs.par) throw invalid_iterator();
            return static_cast<difference_type>(loc - rhs.loc);
        }

        auto operator += (difference_type diff) -> Self& { return loc += diff, *this; }
        auto operator -= (difference_type diff) -> Self& { return loc -= diff, *this; }

        auto operator ++ (int) -> Self { return iterator(loc++, par); }
        auto operator -- (int) -> Self { return iterator(loc--, par); }

        auto operator ++ () -> Self& { return ++loc, *this; }
        auto operator -- () -> Self& { return --loc, *this; }

        auto operator * () const -> reference { return **loc; }
        auto operator -> () const -> pointer { return *loc; }

        auto operator == (const Self &rhs) const -> bool { return loc == rhs.loc; }
        auto operator == (const const_iterator &rhs) const -> bool { return loc == rhs.loc; }

        auto operator != (const Self &rhs) const -> bool { return loc != rhs.loc; }
        auto operator != (const const_iterator &rhs) const -> bool { return loc != rhs.loc; }
    };

    template <typename T>
    class vector<T>::const_iterator {
        using Self	= const_iterator;
        using Up	= vector<T>;

        friend class iterator;
        friend class vector<T>;

        const pointer *loc;
        const Up *par;
        const_iterator(pointer *__loc, const Up *__par): loc(__loc), par(__par) { }

    public:
        const_iterator(): loc(nullptr), par(nullptr) { }
        const_iterator(iterator other): loc(other.loc), par(other.par) { }

        auto operator + (difference_type diff) const -> Self { return const_iterator(loc + diff, par); }
        auto operator - (difference_type diff) const -> Self { return const_iterator(loc - diff, par); }
        // return the distance between two iterators,
        // if these two iterators point to different vectors, throw invaild_iterator.
        auto operator - (const Self &rhs) const -> difference_type {
            if (par != rhs.par) throw invalid_iterator();
            return static_cast<difference_type>(loc - rhs.loc);
        }

        auto operator += (difference_type diff) -> Self& { return loc += diff, *this; }
        auto operator -= (difference_type diff) -> Self& { return loc -= diff, *this; }

        auto operator ++ (int) -> Self { return const_iterator(loc++, par); }
        auto operator -- (int) -> Self { return const_iterator(loc--, par); }

        auto operator ++ () -> Self& { return ++loc, *this; }
        auto operator -- () -> Self& { return --loc, *this; }

        auto operator * () const -> const_reference { return **loc; }
        auto operator -> () const -> const_pointer { return *loc; }

        auto operator == (const Self &rhs) const -> bool { return loc == rhs.loc; }
        auto operator == (const iterator &rhs) const -> bool { return loc == rhs.loc; }

        auto operator != (const Self &rhs) const -> bool { return loc != rhs.loc; }
        auto operator != (const iterator &rhs) const -> bool { return loc != rhs.loc; }
    };





/* impl vector<T> { */

    template <typename T>
    vector<T>::vector(): __beg(nullptr), __end(nullptr), __cap(nullptr) { }

    template <typename T>
    vector<T>::vector(Self &&other)
        : vector() {
            ::std::swap(__beg, other.__beg);
            ::std::swap(__end, other.__end);
            ::std::swap(__cap, other.__cap);
        }

    template <typename T>
    vector<T>::vector(const Self &other)
        : __beg(new pointer[other.capacity()]),
             __end(__beg), __cap(__beg + other.capacity()) {
                 for (pointer *p = other.__beg; p != other.__end; ++p, ++__end)
                     *__end = new value_type(**p);
            }

    template <typename T>
    vector<T>::~vector() {
        for (pointer *p = __beg; p != __end; ++p)
            delete *p;
        delete[] __beg;
    }

    template <typename T>
    auto vector<T>::operator = (Self &&other) -> Self& {
        ::std::swap(__beg, other.__beg);
        ::std::swap(__end, other.__end);
        ::std::swap(__cap, other.__cap);
        return *this;
    }

    template <typename T>
    auto vector<T>::operator = (const Self &other) -> Self& {
        if (this == std::addressof(other)) return *this;
        clear();
        if (capacity() < other.size()) reserve(other.capacity());
        for (pointer *p = other.__beg; p != other.__end; ++p, ++__end)
            *__end = new value_type(**p);
        return *this;
    }

    template <typename T>
    auto vector<T>::clear() -> void {
        for (pointer *p = __beg; p != __end; ++p)
            delete *p;
        __end = __beg;
    }

    template <typename T>
    auto vector<T>::reserve(size_type __size) -> void {
        if (capacity() == 0) {
            __beg = __end = new pointer[__size];
            __cap = __beg + __size;
            return;
        }
        if (__size <= capacity()) return;
        pointer *o_beg = __beg, *o_end = __end;
        __beg = __end = new pointer[__size];
        __cap = __beg + __size;
        for (pointer *p = o_beg; p != o_end; ++p, ++__end)
            *__end = *p;
        delete[] o_beg;
    }

    template <typename T>
    auto vector<T>::at(size_type loc) -> reference {
        if (loc < 0 or size() <= loc)
            throw index_out_of_bound();
        return *__beg[loc];
    }

    template <typename T>
    auto vector<T>::at(size_type loc) const -> const_reference {
        if (loc < 0 or size() <= loc)
            throw index_out_of_bound();
        return *__beg[loc];
    }

    template <typename T>
    auto vector<T>::front() -> reference {
        if (empty()) throw container_is_empty();
        return **__beg;
    }

    template <typename T>
    auto vector<T>::front() const -> const_reference {
        if (empty()) throw container_is_empty();
        return **__beg;
    }

    template <typename T>
    auto vector<T>::back() -> reference {
        if (empty()) throw container_is_empty();
        return **(__end - 1);
    }

    template <typename T>
    auto vector<T>::back() const -> const_reference {
        if (empty()) throw container_is_empty();
        return **(__end - 1);
    }

    template <typename T>
    auto vector<T>::insert(iterator it, const value_type &value) -> iterator {
        if (it.par != this) throw invalid_iterator();
        return insert(it.loc - __beg, value);
    }

    template <typename T>
    auto vector<T>::insert(size_type loc, const value_type &value) -> iterator {
        if (loc < 0 or size() < loc) throw index_out_of_bound();
        if (cap_full()) reserve((size() + 1) * 2);
        for (pointer *p = __end; p != __beg + loc; --p)
            *p = *(p - 1);
        ++__end; __beg[loc] = new value_type(value);
        return iterator(__beg + loc, this);
    }

    template <typename T>
    auto vector<T>::erase(iterator it) -> iterator {
        if (it.par != this) throw invalid_iterator();
        return erase(it.loc - __beg);
    }

    template <typename T>
    auto vector<T>::erase(size_type loc) -> iterator {
        if (loc < 0 or size() <= loc) throw index_out_of_bound();
        delete __beg[loc];
        for (pointer *p = __beg + loc + 1; p != __end; ++p)
            *(p - 1) = *p;
        --__end;
        return iterator(__beg + loc, this);
    }

    template <typename T>
    auto vector<T>::push_back(const value_type &value) -> void {
        if (cap_full()) reserve((size() + 1) * 2);
        *__end++ = new value_type(value);
    }

    template <typename T>
    auto vector<T>::pop_back() -> void {
        if (empty()) throw container_is_empty();
        delete *--__end;
    }

/* } */

}

#endif
