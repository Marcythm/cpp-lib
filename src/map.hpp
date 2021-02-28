/**
 * implement a container like std::map
 */
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include "exceptions.hpp"

#include "rbtree.hpp"

namespace sjtu {

// #define STlite_NOEXCEPT

template <typename T1, typename T2>
class pair {
    using Self = pair;
public:
    using first_type    = T1;
    using second_type   = T2;

    T1 first;
    T2 second;

    constexpr pair(): first(), second() { }
    constexpr pair(const T1 &x, const T2 &y): first(x), second(y) { }
    template <typename U1, typename U2>
        constexpr pair(const U1 &x, const U2 &y)
            : first(x), second(y) { }
    // template <typename U1, typename U2>
    //     constexpr pair(U1 &&x, U2 &&y)
    //         : first(std::move(x)), second(std::move(y)) { }

    template <typename U1, typename U2>
        constexpr pair(const pair<U1, U2> &other)
            : first(other.first), second(other.second) { }
    // template <typename U1, typename U2>
    //     constexpr pair(pair<U1, U2> &&other)
    //         : first(std::move(other.first)), second(std::move(other.second)) { }

    template <typename U1, typename U2>
        constexpr auto operator = (const pair<U1, U2> &other) -> Self&
            { first = other.first; second = other.second; return *this; }
    // template <typename U1, typename U2>
    //     constexpr auto operator = (pair<U1, U2> &&other) -> Self&
    //         { first = std::move(other.first), second = std::move(other.second); return *this; }
};


template <typename Value, typename Ref, typename Ptr, typename Up, typename Up_ptr>
struct map_iterator {
    using Self              = map_iterator;
    using internal_type     = rbtree_iterator<Value, Ref, Ptr>;

    using value_type        = typename internal_type::value_type;
    using reference         = typename internal_type::reference;
    using pointer           = typename internal_type::pointer;
    using difference_type   = typename internal_type::difference_type;
    using iterator_category = typename internal_type::iterator_category;

    using iterator          = map_iterator<value_type, value_type&, value_type*, Up, Up*>;
    using const_iterator    = map_iterator<value_type, const value_type&, const value_type*, Up, const Up*>;

    internal_type it;
    Up_ptr up;

public:
    map_iterator() = default;
    map_iterator(internal_type _it, Up_ptr _up): it(_it), up(_up) {}

    template <typename _Ref, typename _Ptr, typename _Up_ptr>
    map_iterator(map_iterator<Value, _Ref, _Ptr, Up, _Up_ptr> _it): it(_it.it), up(_it.up) {}

    auto operator++ () -> Self& {
        if (it == up->cend().it) throw invalid_iterator();
        ++it; return *this;
    }
    auto operator-- () -> Self& {
        if (it == up->cbegin().it) throw invalid_iterator();
        --it; return *this;
    }

    auto operator++ (i32) -> Self {
        if (it == up->cend().it) throw invalid_iterator();
        return Self(it++, up);
    }
    auto operator-- (i32) -> Self {
        if (it == up->cbegin().it) throw invalid_iterator();
        return Self(it--, up);
    }

    auto operator*  () const -> reference { return *it; }
    auto operator-> () const noexcept -> pointer { return std::addressof(operator*()); }
};

template <typename Val, typename Up, typename Refl, typename Ptrl, typename Up_ptrl, typename Refr, typename Ptrr, typename Up_ptrr>
inline auto operator== (
    const map_iterator<Val, Refl, Ptrl, Up, Up_ptrl> &lhs,
    const map_iterator<Val, Refr, Ptrr, Up, Up_ptrr> &rhs
) -> bool {
    return lhs.up == rhs.up and lhs.it == rhs.it;
}

template <typename Val, typename Up, typename Refl, typename Ptrl, typename Up_ptrl, typename Refr, typename Ptrr, typename Up_ptrr>
inline auto operator!= (
    const map_iterator<Val, Refl, Ptrl, Up, Up_ptrl> &lhs,
    const map_iterator<Val, Refr, Ptrr, Up, Up_ptrr> &rhs
) -> bool {
    return lhs.up != rhs.up or lhs.it != rhs.it;
}


template <class Key, class T, class Compare = std::less<Key>>
class map {
    using Self = map;

public:
    using key_type              = Key;
    using mapped_type           = T;
    using key_compare           = Compare;
    using value_type            = pair<const Key, T>;

private:
    struct KoV {
        auto operator() (const value_type &value) const -> const key_type& {
            return value.first;
        }
    };
    using internal_type         = rbtree<key_type, value_type, KoV, Compare>;

public:
    using reference             = typename internal_type::reference;
    using const_reference       = typename internal_type::const_reference;
    using pointer               = typename internal_type::pointer;
    using const_pointer         = typename internal_type::const_pointer;
    using size_type             = typename internal_type::size_type;
    using difference_type       = typename internal_type::difference_type;

    using iterator              = map_iterator<value_type, reference, pointer, Self, Self*>;
    using const_iterator        = map_iterator<value_type, const_reference, const_pointer, Self, const Self*>;

private:
    internal_type tree;

public:
    map(const Compare &cmp = Compare()): tree(cmp) {}
    map(const Self &other): tree(other.tree) {}
    ~map() = default;

    auto operator = (const Self &other) -> Self& { tree = other.tree; return *this; }

    auto at(const Key &key) -> mapped_type& {
        typename internal_type::iterator it = tree.find(key);
        if (it == tree.end()) {
            throw index_out_of_bound();
        }
        return it->second;
    }

    auto at(const Key &key) const -> const mapped_type& {
        typename internal_type::const_iterator it = tree.find(key);
        if (it == tree.cend()) {
            throw index_out_of_bound();
        }
        return it->second;
    }

    auto operator[](const Key &key) -> mapped_type& {
        typename internal_type::iterator it = tree.find(key);
        if (it == tree.end()) {
            it = tree.insert_unique(value_type(key, mapped_type())).first;
        }
        return it->second;
    }

    auto operator[](const Key &key) const -> const mapped_type& { return at(key); }

    auto begin() -> iterator { return iterator(tree.begin(), this); }
    auto cbegin() const -> const_iterator { return const_iterator(tree.cbegin(), this); }

    auto end() -> iterator { return iterator(tree.end(), this); }
    auto cend() const -> const_iterator { return const_iterator(tree.cend(), this); }

    auto empty() const -> bool { return tree.empty(); }
    auto size() const -> size_type { return tree.size(); }
    auto clear() -> void { return tree.clear(); }

    auto insert(const value_type &value) -> pair<iterator, bool> {
        std::pair<typename internal_type::iterator, bool> res = tree.insert_unique(value);
        return pair<iterator, bool>(iterator(res.first, this), res.second);
    }

    auto erase(iterator pos) -> void {
        if (pos.up != this or pos == end())
            throw invalid_iterator();
        return tree.erase(pos.it);
    }

    auto count(const Key &key) const -> size_type {
        return tree.find(key) == tree.cend() ? 0 : 1;
    }

    auto find(const Key &key) -> iterator {
        return iterator(tree.find(key), this);
    }
    auto find(const Key &key) const -> const_iterator {
        return const_iterator(tree.find(key), this);
    }
};

}

#endif
