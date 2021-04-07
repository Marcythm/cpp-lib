#pragma once
#pragma message("the \"rbtree.hpp\" header included")
#include <algorithm>
#include <memory>
#include <type_traits>

using i8    =   signed char;        // int8_t;
using i16   =   signed short;       // int16_t;
using i32   =   signed int;         // int32_t;
using i64   =   signed long long;   // int64_t;

using u8    =   unsigned char;      // uint8_t;
using u16   =   unsigned short;     // uint16_t;
using u32   =   unsigned int;       // uint32_t;
using u64   =   unsigned long long; // uint64_t;

using f32   =   float;
using f64   =   double;
using f80   =   long double;


enum class rbtree_color_type: bool {
    red   = false,
    black = true,
};


// struct rbtree_node_base {
//     using Self          = rbtree_node_base;
//     using base_ptr      = Self*;
//     using color_type    = rbtree_color_type;

//     color_type color;
//     base_ptr left, right, parent;

//     rbtree_node_base() = default;
//     virtual ~rbtree_node_base() = default;

//     static auto minimum(base_ptr u) -> base_ptr {
//         for (; u->left != nullptr; u = u->left);
//         return u;
//     }

//     static auto maximum(base_ptr u) -> base_ptr {
//         for (; u->right != nullptr; u = u->right);
//         return u;
//     }
// };

template <typename T>
struct rbtree_node/* : rbtree_node_base */ {
    using Self          = rbtree_node;
    using value_type    = T;
    using link_type     = Self*;
    using color_type    = rbtree_color_type;

    color_type color;
    link_type left, right, parent;
    value_type value;

    rbtree_node()
        : left(nullptr), right(nullptr), parent(nullptr) {}
    rbtree_node(const value_type &_value)
        : left(nullptr), right(nullptr), parent(nullptr), value(_value) {}

    ~rbtree_node() = default;

    static auto minimum(link_type u) -> link_type {
        for (; u->left != nullptr; u = u->left);
        return u;
    }

    static auto maximum(link_type u) -> link_type {
        for (; u->right != nullptr; u = u->right);
        return u;
    }
};


// struct rbtree_iterator_base {
//     using Self          = rbtree_iterator_base;
//     using base_ptr      = rbtree_node_base::base_ptr;
//     using color_type    = rbtree_color_type;

//     base_ptr u;

//     ~rbtree_iterator_base() = default;

//     auto inc() -> void {
//         if (u->right != nullptr) {
//             u = u->right;
//             while (u->left != nullptr)
//                 u = u->left;
//         } else {
//             base_ptr p = u->parent;
//             while (u == p->right) {
//                 u = p;
//                 p = p->parent;
//             }
//             if (u->right != p)
//                 u = p;
//         }
//     }

//     auto dec() -> void {
//         if (u->color == color_type::red and u->parent->parent == u)
//             u = u->right;
//         else if (u->left != nullptr) {
//             u = u->left;
//             while (u->right != nullptr)
//                 u = u->right;
//         } else {
//             base_ptr p = u->parent;
//             while (u == p->left) {
//                 u = p;
//                 p = p->parent;
//             }
//             u = p;
//         }
//     }

//     friend inline auto operator== (const Self &x, const Self &y) -> bool {
//         return x.u == y.u;
//     }
//     friend inline auto operator!= (const Self &x, const Self &y) -> bool {
//         return x.u != y.u;
//     }
// };

template <typename Value, typename Ref, typename Ptr>
struct rbtree_iterator/* : rbtree_iterator_base */ {
    using Self              = rbtree_iterator;
    using link_type         = rbtree_node<Value>*;
    using color_type        = rbtree_color_type;

    using value_type        = Value;
    using reference         = Ref;
    using pointer           = Ptr;
    using difference_type   = std::ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;

    using iterator          = rbtree_iterator<value_type, value_type&, value_type*>;
    using const_iterator    = rbtree_iterator<value_type, const value_type&, const value_type*>;

    link_type u;

    rbtree_iterator() = default;
    rbtree_iterator(link_type p) { u = p; }
    rbtree_iterator(const iterator &it) { u = it.u; }

    ~rbtree_iterator() = default;

    auto operator*  () const -> reference { return link_type(u)->value; }
    auto operator-> () const -> pointer { return std::addressof(operator*()); }

    auto inc() -> void {
        if (u->right != nullptr) {
            u = u->right;
            while (u->left != nullptr)
                u = u->left;
        } else {
            link_type p = u->parent;
            while (u == p->right) {
                u = p;
                p = p->parent;
            }
            if (u->right != p)
                u = p;
        }
    }

    auto dec() -> void {
        if (u->color == color_type::red and u->parent->parent == u) {
            u = u->right;
        } else if (u->left != nullptr) {
            u = u->left;
            while (u->right != nullptr)
                u = u->right;
        } else {
            link_type p = u->parent;
            while (u == p->left) {
                u = p;
                p = p->parent;
            }
            u = p;
        }
    }

    auto operator++ () -> Self& { inc(); return *this; }
    auto operator-- () -> Self& { dec(); return *this; }

    auto operator++ (i32) -> Self { Self t = *this; inc(); return t; }
    auto operator-- (i32) -> Self { Self t = *this; dec(); return t; }
};

template <typename Val, typename Refl, typename Ptrl, typename Refr, typename Ptrr>
inline auto operator== (
    const rbtree_iterator<Val, Refl, Ptrl> &lhs,
    const rbtree_iterator<Val, Refr, Ptrr> &rhs
) -> bool {
    return lhs.u == rhs.u;
}

template <typename Val, typename Refl, typename Ptrl, typename Refr, typename Ptrr>
inline auto operator!= (
    const rbtree_iterator<Val, Refl, Ptrl> &lhs,
    const rbtree_iterator<Val, Refr, Ptrr> &rhs
) -> bool {
    return lhs.u != rhs.u;
}


template <typename T>
struct rbtree_helper {
    using node_type     = rbtree_node<T>;
    using link_type     = typename node_type::link_type;
    using color_type    = rbtree_color_type;

    static inline auto rotate_left(link_type u, link_type &root) -> void {
        link_type v = u->right;
        u->right = v->left;
        if (v->left != nullptr)
            v->left->parent = u;
        v->parent = u->parent;

        if (u == root) {
            root = v;
        } else if (u == u->parent->left) {
            u->parent->left = v;
        } else {
            u->parent->right = v;
        }
        v->left = u;
        u->parent = v;
    }

    static inline auto rotate_right(link_type u, link_type &root) -> void {
        link_type v = u->left;
        u->left = v->right;
        if (v->right != nullptr)
            v->right->parent = u;
        v->parent = u->parent;

        if (u == root) {
            root = v;
        } else if (u == u->parent->right) {
            u->parent->right = v;
        } else {
            u->parent->left = v;
        }
        v->right = u;
        u->parent = v;
    }

    static inline auto rebalance_for_insert(link_type u, link_type &root) -> void {
        u->color = color_type::red;
        while (u != root and u->parent->color == color_type::red) {
            if (u->parent == u->parent->parent->left) {
                link_type uncle = u->parent->parent->right;
                if (uncle != nullptr and uncle->color == color_type::red) {
                    u->parent->color = color_type::black;
                    uncle->color = color_type::black;
                    u->parent->parent->color = color_type::red;
                    u = u->parent->parent;
                } else {
                    if (u == u->parent->right) {
                        u = u->parent;
                        rotate_left(u, root);
                    }
                    u->parent->color = color_type::black;
                    u->parent->parent->color = color_type::red;
                    rotate_right(u->parent->parent, root);
                }
            } else {
                link_type uncle = u->parent->parent->left;
                if (uncle != nullptr and uncle->color == color_type::red) {
                    u->parent->color = color_type::black;
                    uncle->color = color_type::black;
                    u->parent->parent->color = color_type::red;
                    u = u->parent->parent;
                } else {
                    if (u == u->parent->left) {
                        u = u->parent;
                        rotate_right(u, root);
                    }
                    u->parent->color = color_type::black;
                    u->parent->parent->color = color_type::red;
                    rotate_left(u->parent->parent, root);
                }
            }
        }
        root->color = color_type::black;
    }

    static inline auto rebalance_for_erase(link_type z, link_type &root, link_type &leftmost, link_type &rightmost) -> link_type {
        link_type y = z;
        link_type x = nullptr;
        link_type p = nullptr;
        if (y->left == nullptr) {
            x = y->right;
        } else if (y->right == nullptr) {
            x = y->left;
        } else {
            y = y->right;
            while (y->left != nullptr)
                y = y->left;
            x = y->right;
        }

        if (y != z) {
            z->left->parent = y;
            y->left = z->left;

            if (y != z->right) {
                p = y->parent;
                if (x != nullptr) {
                    x->parent = y->parent;
                }
                y->parent->left = x;
                y->right = z->right;
                z->right->parent = y;
            } else {
                p = y;
            }

            if (root == z) {
                root = y;
            } else if (z->parent->left == z) {
                z->parent->left = y;
            } else {
                z->parent->right = y;
            }

            y->parent = z->parent;
            std::swap(y->color, z->color);
            y = z;
        } else {
            p = y->parent;
            if (x != nullptr) {
                x->parent = y->parent;
            }

            if (root == z) {
                root = x;
            } else if (z->parent->left == z) {
                z->parent->left = x;
            } else {
                z->parent->right = x;
            }

            if (leftmost == z) {
                if (z->right == nullptr) {
                    leftmost = z->parent;
                } else {
                    leftmost = node_type::minimum(x);
                }
            }

            if (rightmost == z) {
                if (z->left == nullptr) {
                    rightmost = z->parent;
                } else {
                    rightmost = node_type::maximum(x);
                }
            }
        }

        if (y->color != color_type::red) {
            while (x != root and (x == nullptr or x->color == color_type::black)) {
                if (x == p->left) {
                    link_type sibling = p->right;
                    if (sibling->color == color_type::red) {
                        sibling->color = color_type::black;
                        p->color = color_type::red;
                        rotate_left(p, root);
                        sibling = p->right;
                    }
                    if (
                        (sibling->left == nullptr or sibling->left->color == color_type::black)
                    and (sibling->right == nullptr or sibling->right->color == color_type::black)
                    ) {
                        sibling->color = color_type::red;
                        x = p;
                        p = p->parent;
                    } else {
                        if (sibling->right == nullptr or sibling->right->color == color_type::black) {
                            if (sibling->left != nullptr) {
                                sibling->left->color = color_type::black;
                            }
                            sibling->color = color_type::red;
                            rotate_right(sibling, root);
                            sibling = p->right;
                        }
                        sibling->color = p->color;
                        p->color = color_type::black;
                        if (sibling->right != nullptr) {
                            sibling->right->color = color_type::black;
                        }
                        rotate_left(p, root);
                        break;
                    }
                } else {                  // same as above, with right <-> left.
                    link_type sibling = p->left;
                    if (sibling->color == color_type::red) {
                        sibling->color = color_type::black;
                        p->color = color_type::red;
                        rotate_right(p, root);
                        sibling = p->left;
                    }
                    if (
                        (sibling->right == nullptr or sibling->right->color == color_type::black)
                    and (sibling->left == nullptr or sibling->left->color == color_type::black)
                    ) {
                        sibling->color = color_type::red;
                        x = p;
                        p = p->parent;
                    } else {
                        if (sibling->left == nullptr or sibling->left->color == color_type::black) {
                            if (sibling->right != nullptr) {
                                sibling->right->color = color_type::black;
                            }
                            sibling->color = color_type::red;
                            rotate_left(sibling, root);
                            sibling = p->left;
                        }
                        sibling->color = p->color;
                        p->color = color_type::black;
                        if (sibling->left != nullptr) {
                            sibling->left->color = color_type::black;
                        }
                        rotate_right(p, root);
                        break;
                    }
                }
            }
            if (x != nullptr) {
                x->color = color_type::black;
            }
        }
        return y;
    }
};


template <typename Key, typename Value, typename KeyOfValue, typename Compare = std::less<Key>>
class rbtree {
    using Self              = rbtree;
    using helper            = rbtree_helper<Value>;
    // using base_ptr          = rbtree_node_base::base_ptr;
    using node_type         = typename helper::node_type;
    using link_type         = typename helper::link_type;
    using color_type        = typename helper::color_type;

    static_assert(std::is_convertible_v<KeyOfValue, std::function<Key(Value)>>);

public:
    using key_type          = Key;
    using value_type        = Value;
    using pointer           = value_type*;
    using const_pointer     = const value_type*;
    using reference         = value_type&;
    using const_reference   = const value_type&;

    using size_type         = size_t;
    using difference_type   = std::ptrdiff_t;

    using iterator          = rbtree_iterator<value_type, reference, pointer>;
    using const_iterator    = rbtree_iterator<value_type, const_reference, const_pointer>;

public:
    auto get_node() -> link_type { return static_cast<link_type>(std::malloc(sizeof(node_type))); }
    auto put_node(link_type p) -> void { return std::free(p); }

    auto new_node(const value_type &value) -> link_type {
        return new(get_node()) node_type(value);
    }
    auto del_node(link_type p) -> void {
        p->value.~value_type();
        put_node(p);
    }

    auto clone_node(link_type u) -> link_type {
        link_type v = new_node(u->value);
        v->color = u->color;
        return v;
    }

private:
    size_type node_count;
    link_type header;
    Compare key_compare;

    auto root()      const -> link_type& { return header->parent; }
    auto leftmost()  const -> link_type& { return header->left;   }
    auto rightmost() const -> link_type& { return header->right;  }

    static auto minimum(link_type u) -> link_type { return node_type::minimum(u); }
    static auto maximum(link_type u) -> link_type { return node_type::maximum(u); }

private:
    auto __copy(link_type u, link_type p) -> link_type;
    auto __insert(link_type u, link_type p, const value_type &value) -> iterator;
    /// erase recursively
    auto __erase(link_type u) -> void;

    auto init() -> void {
        header = get_node();
        header->color = color_type::red;

        root() = nullptr;
        leftmost() = header;
        rightmost() = header;
    }

public:
    rbtree(const Compare &cmp = Compare())
        : node_count(0), key_compare(cmp) { init(); }

    rbtree(const Self &other)
        : node_count(other.node_count), key_compare(other.key_compare) {
            header = get_node();
            header->color = color_type::red;
            if (other.root() == nullptr) {
                root() = nullptr;
                leftmost() = header;
                rightmost() = header;
            } else {
                root() = __copy(other.root(), header);
                leftmost() = minimum(root());
                rightmost() = maximum(root());
            }
        }

    ~rbtree() {
        clear();
        put_node(header);
    }

    auto operator= (const Self &other) -> Self&;

public:
    auto key_comp() const -> Compare { return key_compare; }

    auto begin() -> iterator { return leftmost(); }
    auto cbegin() const -> const_iterator { return leftmost(); }

    auto end() -> iterator { return header; }
    auto cend() const -> const_iterator { return header; }

    auto empty() const -> bool { return node_count == 0; }
    auto size() const -> size_type { return node_count; }

    auto swap(Self &other) -> void {
        std::swap(node_count, other.node_count);
        std::swap(header, other.header);
        std::swap(key_compare, other.key_compare);
    }

public:
    auto insert_unique(const value_type &value) -> std::pair<iterator, bool>;
    auto insert_unique(iterator pos, const value_type &value) -> iterator;
    auto insert_unique(const_iterator first, const_iterator last) -> void;
    auto insert_unique(const value_type *first, const value_type *last) -> void;

    auto insert_equal(const value_type &value) -> iterator;
    auto insert_equal(iterator pos, const value_type &value) -> iterator;
    auto insert_equal(const_iterator first, const_iterator last) -> void;
    auto insert_equal(const value_type *first, const value_type *last) -> void;

    auto erase(iterator pos) -> void;
    auto erase(const key_type &key) -> size_type;
    auto erase(iterator first, iterator last) -> void;
    auto erase(const key_type *first, const key_type *last) -> void;

    auto clear() -> void {
        if (node_count != 0) {
            __erase(root());
            root() = nullptr;
            leftmost() = header;
            rightmost() = header;
            node_count = 0;
        }
    }

public:
    auto count(const key_type &key) const -> size_type;

    auto find(const key_type &key) -> iterator;
    auto find(const key_type &key) const -> const_iterator;

    auto lower_bound(const key_type &key) -> iterator;
    auto lower_bound(const key_type &key) const -> const_iterator;

    auto upper_bound(const key_type &key) -> iterator;
    auto upper_bound(const key_type &key) const -> const_iterator;

    auto equal_range(const key_type &key) -> std::pair<iterator, iterator>;
    auto equal_range(const key_type &key) const -> std::pair<const_iterator, const_iterator>;

public:
    auto verify() const -> bool;

public:
    friend inline auto operator== (const Self &lhs, const Self &rhs) -> bool {
        return lhs.size() == rhs.size() and std::equal(lhs.begin(), lhs.end(), rhs.begin());
    }

    friend inline auto operator< (const Self &lhs, const Self &rhs) -> bool {
        return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }
};


/* impl rbtree<Key, Value, KeyOfValue, Compare> { */

    template <typename Key, typename Value, typename KeyOfValue, typename Compare>
    auto rbtree<Key, Value, KeyOfValue, Compare>::__copy(link_type u, link_type p) -> link_type {
        link_type top = clone_node(u);
        top->parent = p;

        if (u->right != nullptr)
            top->right = __copy(u->right, top);
        p = top;
        u = u->left;

        while (u != nullptr) {
            link_type nu = clone_node(u);
            p->left = nu;
            nu->parent = p;
            if (u->right != nullptr)
                nu->right = __copy(u->right, nu);
            p = nu;
            u = u->left;
        }

        return top;
    }

    template <typename Key, typename Value, typename KeyOfValue, typename Compare>
    auto rbtree<Key, Value, KeyOfValue, Compare>::__insert(link_type u, link_type p, const value_type &value) -> iterator {
        link_type nv = new_node(value);

        if (p == header or u != nullptr or key_compare(KeyOfValue()(value), KeyOfValue()(p->value))) {
            p->left = nv;
            if (p == header) {
                root() = nv;
                rightmost() = nv;
            } else if (p == leftmost()) {
                leftmost() = nv;
            }
        } else {
            p->right = nv;
            if (p == rightmost()) {
                rightmost() = nv;
            }
        }

        nv->parent = p;
        nv->left = nullptr;
        nv->right = nullptr;

        helper::rebalance_for_insert(nv, root());
        ++node_count;
        return iterator(nv);
    }

    template <typename Key, typename Value, typename KeyOfValue, typename Compare>
    auto rbtree<Key, Value, KeyOfValue, Compare>::__erase(link_type u) -> void {
        while (u != nullptr) {
            __erase(u->right);
            link_type v = u->left;
            del_node(u);
            u = v;
        }
    }

/* =========================================================================== */

    template <typename Key, typename Value, typename KeyOfValue, typename Compare>
    auto rbtree<Key, Value, KeyOfValue, Compare>::operator= (const Self &other) -> Self& {
        if (this != std::addressof(other)) {
            clear();
            node_count = 0;
            key_compare = other.key_compare;
            if (other.root() == nullptr) {
                root() = nullptr;
                leftmost() = header;
                rightmost() = header;
            } else {
                root() = __copy(other.root(), header);
                leftmost() = minimum(root());
                rightmost() = maximum(root());
                node_count = other.node_count;
            }
        } return *this;
    }

/* =========================================================================== */

    template <typename Key, typename Value, typename KeyOfValue, typename Compare>
    auto rbtree<Key, Value, KeyOfValue, Compare>::insert_unique(const value_type &value) -> std::pair<iterator, bool> {
        link_type p = header;
        link_type u = root();
        bool comp = true;
        while (u != nullptr) {
            p = u;
            comp = key_compare(KeyOfValue()(value), KeyOfValue()(u->value));
            u = comp ? u->left : u->right;
        }

        iterator pos(p);
        if (comp) {
            if (pos == begin()) {
                return std::pair<iterator, bool>(__insert(u, p, value), true);
            } else {
                --pos;
            }
        }

        if (key_compare(KeyOfValue()(*pos), KeyOfValue()(value)))
            return std::pair<iterator, bool>(__insert(u, p, value), true);

        return std::pair<iterator, bool>(pos, false);
    }

    template <typename Key, typename Value, typename KeyOfValue, typename Compare>
    auto rbtree<Key, Value, KeyOfValue, Compare>::insert_unique(iterator pos, const value_type &value) -> iterator {
        if (pos == begin()) {
            if (size() > 0 and key_compare(KeyOfValue()(value), KeyOfValue()(*pos))) {
                return __insert(pos.u, pos.u, value);
            } else {
                return insert_unique(value).first;
            }
        } else if (pos == end()) {
            if (key_compare(KeyOfValue()(*rightmost()), KeyOfValue()(value))) {
                return __insert(nullptr, rightmost(), value);
            } else {
                return insert_unique(value).first;
            }
        } else {
            iterator before = pos; --before;
            if (key_compare(KeyOfValue()(*before), KeyOfValue()(value)) and key_compare(KeyOfValue()(value), KeyOfValue()(*before))) {
                if (before.u->right == nullptr) {
                    return __insert(nullptr, before.u, value);
                } else {
                    return __insert(pos.u, pos.u, value);
                }
            } else {
                return insert_unique(value).first;
            }
        }
    }


    template <typename Key, typename Value, typename KeyOfValue, typename Compare>
    auto rbtree<Key, Value, KeyOfValue, Compare>::insert_equal(const value_type &value) -> iterator {
        link_type p = header;
        link_type u = root();
        while (u != nullptr) {
            p = u;
            u = key_compare(KeyOfValue()(value), KeyOfValue()(u->value)) ? u->left : u->right;
        }
        return __insert(u, p, value);
    }

    template <typename Key, typename Value, typename KeyOfValue, typename Compare>
    auto rbtree<Key, Value, KeyOfValue, Compare>::insert_equal(iterator pos, const value_type &value) -> iterator {
        if (pos == begin()) {
            if (size() > 0 and key_compare(KeyOfValue()(value), KeyOfValue()(*pos))) {
                return __insert(pos.u, pos.u, value);
            } else {
                return insert_equal(value);
            }
        } else if (pos == end()) {
            if (key_compare(KeyOfValue()(value), KeyOfValue()(*rightmost()))) {
                return insert_equal(value);
            } else {
                return __insert(nullptr, rightmost(), value);
            }
        } else {
            iterator before = pos; --before;
            if (!key_compare(KeyOfValue()(value), KeyOfValue()(*before)) and !key_compare(KeyOfValue()(*pos), KeyOfValue()(value))) {
                if (before.u->right == nullptr) {
                    return __insert(nullptr, before.u, value);
                } else {
                    return __insert(pos.u, pos.u, value);
                }
            } else {
                return insert_equal(value);
            }
        }
    }

    template <typename Key, typename Value, typename KeyOfValue, typename Compare>
    auto rbtree<Key, Value, KeyOfValue, Compare>::erase(iterator pos) -> void {
        del_node(helper::rebalance_for_erase(pos.u, root(), leftmost(), rightmost()));
        --node_count;
    }

    template <typename Key, typename Value, typename KeyOfValue, typename Compare>
    auto rbtree<Key, Value, KeyOfValue, Compare>::erase(const key_type &key) -> size_type {
        iterator first = lower_bound(key);
        iterator last = upper_bound(key);
        const size_type n = std::distance(first, last);
        erase(first, last);
        return n;
    }

/* =========================================================================== */

    template <typename Key, typename Value, typename KeyOfValue, typename Compare>
    auto rbtree<Key, Value, KeyOfValue, Compare>::insert_unique(const_iterator first, const_iterator last) -> void {
        for ( ; first != last; ++first)
            insert_unique(*first);
    }

    template <typename Key, typename Value, typename KeyOfValue, typename Compare>
    auto rbtree<Key, Value, KeyOfValue, Compare>::insert_unique(const value_type *first, const value_type *last) -> void {
        for ( ; first != last; ++first)
            insert_unique(*first);
    }

    template <typename Key, typename Value, typename KeyOfValue, typename Compare>
    auto rbtree<Key, Value, KeyOfValue, Compare>::insert_equal(const_iterator first, const_iterator last) -> void {
        for ( ; first != last; ++first)
            insert_equal(*first);
    }

    template <typename Key, typename Value, typename KeyOfValue, typename Compare>
    auto rbtree<Key, Value, KeyOfValue, Compare>::insert_equal(const value_type *first, const value_type *last) -> void {
        for ( ; first != last; ++first)
            insert_equal(*first);
    }

    template <typename Key, typename Value, typename KeyOfValue, typename Compare>
    auto rbtree<Key, Value, KeyOfValue, Compare>::erase(iterator first, iterator last) -> void {
        if (first == begin() and last == end()) {
            clear();
        } else {
            for ( ; first != last; ++first)
                erase(first);
        }
    }

    template <typename Key, typename Value, typename KeyOfValue, typename Compare>
    auto rbtree<Key, Value, KeyOfValue, Compare>::erase(const key_type *first, const key_type *last) -> void {
        for ( ; first != last; ++first)
            erase(*first);
    }

/* =========================================================================== */

    template <typename Key, typename Value, typename KeyOfValue, typename Compare>
    auto rbtree<Key, Value, KeyOfValue, Compare>::count(const key_type &key) const -> size_type {
        return std::distance(lower_bound(key), upper_bound(key));
    }

    template <typename Key, typename Value, typename KeyOfValue, typename Compare>
    auto rbtree<Key, Value, KeyOfValue, Compare>::find(const key_type &key) -> iterator {
        link_type p = header;
        link_type u = root();
        while (u != nullptr) {
            if (key_compare(KeyOfValue()(u->value), key)) {
                u = u->right;
            } else {
                p = u;
                u = u->left;
            }
        }
        const iterator pos(p);
        return (pos == end() or key_compare(key, KeyOfValue()(*pos))) ? end() : pos;
    }

    template <typename Key, typename Value, typename KeyOfValue, typename Compare>
    auto rbtree<Key, Value, KeyOfValue, Compare>::find(const key_type &key) const -> const_iterator {
        link_type p = header;
        link_type u = root();
        while (u != nullptr) {
            if (key_compare(KeyOfValue()(u->value), key)) {
                u = u->right;
            } else {
                p = u;
                u = u->left;
            }
        }
        const const_iterator pos(p);
        return (pos == cend() or key_compare(key, KeyOfValue()(*pos))) ? cend() : pos;
    }

    template <typename Key, typename Value, typename KeyOfValue, typename Compare>
    auto rbtree<Key, Value, KeyOfValue, Compare>::lower_bound(const key_type &key) -> iterator {
        link_type p = header;
        link_type u = root();
        while (u != nullptr) {
            if (key_compare(KeyOfValue()(u->value), key)) {
                u = u->right;
            } else {
                p = u;
                u = u->left;
            }
        }
        return iterator(p);
    }

    template <typename Key, typename Value, typename KeyOfValue, typename Compare>
    auto rbtree<Key, Value, KeyOfValue, Compare>::lower_bound(const key_type &key) const -> const_iterator {
        link_type p = header;
        link_type u = root();
        while (u != nullptr) {
            if (key_compare(KeyOfValue()(u->value), key)) {
                u = u->right;
            } else {
                p = u;
                u = u->left;
            }
        }
        return const_iterator(p);
    }

    template <typename Key, typename Value, typename KeyOfValue, typename Compare>
    auto rbtree<Key, Value, KeyOfValue, Compare>::upper_bound(const key_type &key) -> iterator {
        link_type p = header;
        link_type u = root();
        while (u != nullptr) {
            if (key_compare(key, KeyOfValue()(u->value))) {
                p = u;
                u = u->left;
            } else {
                u = u->right;
            }
        }
        return iterator(p);
    }

    template <typename Key, typename Value, typename KeyOfValue, typename Compare>
    auto rbtree<Key, Value, KeyOfValue, Compare>::upper_bound(const key_type &key) const -> const_iterator {
        link_type p = header;
        link_type u = root();
        while (u != nullptr) {
            if (key_compare(key, KeyOfValue()(u->value))) {
                p = u;
                u = u->left;
            } else {
                u = u->right;
            }
        }
        return const_iterator(p);
    }

    template <typename Key, typename Value, typename KeyOfValue, typename Compare>
    auto rbtree<Key, Value, KeyOfValue, Compare>::equal_range(const key_type &key) -> std::pair<iterator, iterator> {
        return std::pair<iterator, iterator>(lower_bound(key), upper_bound(key));
    }

    template <typename Key, typename Value, typename KeyOfValue, typename Compare>
    auto rbtree<Key, Value, KeyOfValue, Compare>::equal_range(const key_type &key) const -> std::pair<const_iterator, const_iterator> {
        return std::pair<const_iterator, const_iterator>(lower_bound(key), upper_bound(key));
    }

/* =========================================================================== */

    template <typename Key, typename Value, typename KeyOfValue, typename Compare>
    inline auto black_count(
        typename rbtree<Key, Value, KeyOfValue, Compare>::link_type u,
        typename rbtree<Key, Value, KeyOfValue, Compare>::link_type root
    ) -> i32 {
        if (u == nullptr)
            return 0;
        return (u->color == rbtree<Key, Value, KeyOfValue, Compare>::color_type::black ? 1 : 0)
             + (u == root ? 0 : black_count(u->parent, root));
    }

    template <typename Key, typename Value, typename KeyOfValue, typename Compare>
    auto rbtree<Key, Value, KeyOfValue, Compare>::verify() const -> bool {
        if (node_count == 0 or begin() == end())
            return node_count == 0 and begin() == end()
                and header->left == header and header->right == header;

        const i32 len = black_count(leftmost(), root());
        for (const_iterator it = cbegin(); it != cend(); ++it) {
            link_type u = it.u;
            link_type L = u->left;
            link_type R = u->right;

            if (u->color == color_type::red) {
                if (
                    (L != nullptr and L->color == color_type::red)
                 or (R != nullptr and R->color == color_type::red)
                ) return false;
            }

            if (L != nullptr and key_compare(KeyOfValue()(u->value), KeyOfValue()(L->value)))
                return false;
            if (R != nullptr and key_compare(KeyOfValue()(u->value), KeyOfValue()(R->value)))
                return false;

            if (L == nullptr and R == nullptr and black_count(u, root()) != len)
                return false;
        }

        if (leftmost() != minimum(root()))
            return false;
        if (rightmost() != maximum(root()))
            return false;

        return true;
    }

/* } */
