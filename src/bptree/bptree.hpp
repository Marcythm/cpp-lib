#pragma once

#include "config.hpp"
#include "HardDiskSupport/IO.hpp"
#include "HardDiskSupport/Record.hpp"

namespace __cpplib {

using namespace __config;

template <typename Key, typename Value, typename Compare = std::less<Key>, i32 FACTOR = 100>
class bptree {
    static_assert(FACTOR > 3, "FACTOR of bptree too small");

    using Self              = bptree;

public:
    using key_type          = Key;
    using value_type        = Value;
    using key_compare       = Compare;
    using size_type         = size_t;
    using difference_type   = ::std::ptrdiff_t;

    class iterator;

private:
    struct leaf_node;
    struct internal_node;

    struct header_type {
        HardDisk::Record root;
    } header;

    key_compare key_le;
    auto key_eq(const key_type &lhs, const key_type &rhs) const -> bool { return not (key_le(lhs, rhs) or key_le(rhs, lhs)); }

    HardDisk::IO datafile, indexfile;
    HardDisk::RecordPool<value_type> datapool;
    HardDisk::RecordPool<leaf_node> leaf_node_pool;
    HardDisk::RecordPool<internal_node> internal_node_pool;
    internal_node *root;

public:
    bptree(const str & = str("data.bin"), const str & = str("index.bin"));
    ~bptree();

    auto end() const -> iterator { return iterator(const_cast<Self*>(this), leaf_node(), -1); }

private:
    auto insert (leaf_node &u, const key_type &key, const value_type &value) -> std::pair<iterator, bool>;
    auto erase  (leaf_node &u, const key_type &key) -> bool;
    auto find   (leaf_node &u, const key_type &key) -> iterator;
    auto value  (leaf_node &u, const key_type &key) -> value_type;

    auto insert (internal_node &u, const key_type &key, const value_type &value) -> std::pair<iterator, bool>;
    auto erase  (internal_node &u, const key_type &key) -> bool;
    auto find   (internal_node &u, const key_type &key) -> iterator;
    auto value  (internal_node &u, const key_type &key) -> value_type;

public:
    auto insert (const key_type &key, const value_type &value) -> iterator;
    auto erase  (const key_type &key) -> void;
    auto find   (const key_type &key) -> iterator;
    auto value  (const key_type &key) -> value_type;
};


template <typename Key, typename Value, typename Compare, i32 FACTOR>
struct bptree<Key, Value, Compare, FACTOR>::leaf_node {
    static constexpr i32 MIN_KEY_NUM = (FACTOR - 1) / 2 - 1;
    static constexpr i32 MAX_KEY_NUM = FACTOR - 1;
    static constexpr i32 MAX_REC_NUM = MAX_KEY_NUM;

    size_type           size;
    HardDisk::Record    left, right;
    key_type            key[MAX_KEY_NUM + 1];
    HardDisk::Record    rec[MAX_REC_NUM + 1];

    auto scanty () const -> bool { return size < MIN_KEY_NUM; }
    auto surplus() const -> bool { return size > MIN_KEY_NUM; }
    auto full   () const -> bool { return size > MAX_KEY_NUM; }

    leaf_node(): size(0) {}
};

/* impl bptree<Key, Value, Compare, FACTOR>::leaf_node { */

    template <typename Key, typename Value, typename Compare, i32 FACTOR>
    auto bptree<Key, Value, Compare, FACTOR>::insert(leaf_node &u, const key_type &key, const value_type &value) -> std::pair<iterator, bool> {
        size_type loc = std::lower_bound(u.key, u.key + u.size, key, key_le) - u.key;
        if (loc < u.size and key_eq(key, u.key[loc]))
            u.rec[loc].save(datafile, value);
        else {
            std::move_backward(u.key + loc, u.key + u.size, u.key + u.size + 1);
            std::move_backward(u.rec + loc, u.rec + u.size, u.rec + u.size + 1);
            u.key[loc] = key;
            u.rec[loc] = datapool.alloc().save(datafile, value);
            ++u.size;
            return std::make_pair(iterator(this, u, loc), true);
        } return std::make_pair(iterator(this, u, loc), false);
    }

    template <typename Key, typename Value, typename Compare, i32 FACTOR>
    auto bptree<Key, Value, Compare, FACTOR>::erase(leaf_node &u, const key_type &key) -> bool {
        size_type loc = std::lower_bound(u.key, u.key + u.size, key, key_le) - u.key;
        if (loc < u.size and key_eq(key, u.key[loc])) {
            datapool.dealloc(u.rec[loc]);
            std::move(u.key + loc + 1, u.key + u.size, u.key + loc);
            std::move(u.rec + loc + 1, u.rec + u.size, u.rec + loc);
            --u.size;
            return true;
        } return false;
    }

    template <typename Key, typename Value, typename Compare, i32 FACTOR>
    auto bptree<Key, Value, Compare, FACTOR>::find(leaf_node &u, const key_type &key) -> iterator {
        size_type loc = std::lower_bound(u.key, u.key + u.size, key, key_le) - u.key;
        if (loc < u.size and key_eq(key, u.key[loc]))
            return iterator(this, u, loc);
        return iterator();
    }

    template <typename Key, typename Value, typename Compare, i32 FACTOR>
    auto bptree<Key, Value, Compare, FACTOR>::value(leaf_node &u, const key_type &key) -> value_type {
        size_type loc = std::lower_bound(u.key, u.key + u.size, key, key_le) - u.key;
        if (loc < u.size and key_eq(key, u.key[loc]))
            return u.rec[loc].template get<value_type>(datafile);
        throw "in bptree::value(): key not found";
    }

/* } */


template <typename Key, typename Value, typename Compare, i32 FACTOR>
struct bptree<Key, Value, Compare, FACTOR>::internal_node {
    static constexpr i32 MIN_KEY_NUM = (FACTOR - 1) / 2 - 1;
    static constexpr i32 MAX_KEY_NUM = FACTOR - 1;
    static constexpr i32 MAX_SUB_NUM = MAX_KEY_NUM + 1;

    bool                sub_is_leaf;
    size_type           size;
    key_type            key[MAX_KEY_NUM + 1];
    HardDisk::Record    sub[MAX_SUB_NUM + 1];

    auto scanty () const -> bool { return size < MIN_KEY_NUM; }
    auto surplus() const -> bool { return size > MIN_KEY_NUM; }
    auto full   () const -> bool { return size > MAX_KEY_NUM; }

    internal_node(): sub_is_leaf(false), size(0) {}
};

/* impl bptree<Key, Value, Compare, FACTOR>::internal_node { */

    template <typename Key, typename Value, typename Compare, i32 FACTOR>
    auto bptree<Key, Value, Compare, FACTOR>::insert(internal_node &u, const key_type &key, const value_type &value) -> std::pair<iterator, bool> {
        size_type loc = std::upper_bound(u.key, u.key + u.size, key, key_le) - u.key;
        std::pair<iterator, bool> result;
        if (u.sub_is_leaf) {
            leaf_node *v = static_cast<leaf_node*>(std::malloc(sizeof(leaf_node)));
            u.sub[loc].load(indexfile, *v);
            result = insert(*v, key, value);

            /* if full then split */
            if (v->full()) {
                leaf_node *w = static_cast<leaf_node*>(std::malloc(sizeof(leaf_node)));
                std::move(v->key + (FACTOR / 2), v->key + v->size, w->key);
                std::move(v->rec + (FACTOR / 2), v->rec + v->size, w->rec);
                w->size = v->size - (FACTOR / 2);
                v->size = (FACTOR / 2);

                std::move_backward(u.key + loc,     u.key + u.size,     u.key + u.size + 1);
                std::move_backward(u.sub + loc + 1, u.sub + u.size + 1, u.sub + u.size + 2);
                u.key[loc] = w->key[0];
                ++u.size;


                w->left = u.sub[loc];
                w->right = std::move(v->right);
                u.sub[loc + 1] = leaf_node_pool.alloc().save(indexfile, *w);

                v->right = u.sub[loc + 1];
                u.sub[loc].save(indexfile, *v);

                if (not w->right.empty()) {
                    leaf_node *t = static_cast<leaf_node*>(std::malloc(sizeof(leaf_node)));
                    w->right.load(indexfile, *t);
                    t->left = v->right;
                    w->right.save(indexfile, *t);
                    std::free(t);
                }

                std::free(v); std::free(w);
                return result.second = true, result;
            }
            if (result.second)
                u.sub[loc].save(indexfile, *v),
                result.second = false;
            std::free(v);
        } else {
            internal_node *v = static_cast<internal_node*>(std::malloc(sizeof(internal_node)));
            u.sub[loc].load(indexfile, *v);
            result = insert(*v, key, value);

            /* if full then split */
            if (v->full()) {
                internal_node *w = static_cast<internal_node*>(std::malloc(sizeof(internal_node)));
                std::move(v->key + (FACTOR / 2) + 1, v->key + v->size,     w->key);
                std::move(v->sub + (FACTOR / 2) + 1, v->sub + v->size + 1, w->sub);
                w->size = v->size - (FACTOR / 2) - 1;
                v->size = (FACTOR / 2);
                w->sub_is_leaf = v->sub_is_leaf;

                std::move_backward(u.key + loc,     u.key + u.size,     u.key + u.size + 1);
                std::move_backward(u.sub + loc + 1, u.sub + u.size + 1, u.sub + u.size + 2);
                u.key[loc] = std::move(v->key[(FACTOR / 2)]);
                ++u.size;

                u.sub[loc].save(indexfile, *v);
                u.sub[loc + 1] = internal_node_pool.alloc().save(indexfile, *w);
                std::free(v); std::free(w);
                return result.second = true, result;
            }
            if (result.second)
                u.sub[loc].save(indexfile, *v),
                result.second = false;
            std::free(v);
        } return result;
    }

    template <typename Key, typename Value, typename Compare, i32 FACTOR>
    auto bptree<Key, Value, Compare, FACTOR>::erase(internal_node &u, const key_type &key) -> bool {
        size_type loc = std::upper_bound(u.key, u.key + u.size, key, key_le) - u.key;
        if (u.sub_is_leaf) {
            leaf_node *v = static_cast<leaf_node*>(std::malloc(sizeof(leaf_node)));
            u.sub[loc].load(indexfile, *v);
            bool result = erase(*v, key);

            if (v->scanty()) {
                if (0 < loc) {
                    leaf_node *w = static_cast<leaf_node*>(std::malloc(sizeof(leaf_node)));
                    u.sub[loc - 1].load(indexfile, *w);

                    if (w->surplus()) {
                        /* get key from surplus brothers */
                        std::move_backward(v->key, v->key + v->size, v->key + v->size + 1);
                        std::move_backward(v->rec, v->rec + v->size, v->rec + v->size + 1);
                        --w->size;
                        v->key[0] = std::move(w->key[w->size]);
                        v->rec[0] = std::move(w->rec[w->size]);
                        ++v->size;

                        u.key[loc - 1] = v->key[0];
                        u.sub[loc].save(indexfile, *v);
                    } else {
                        /* merge with brothers */
                        std::move(v->key, v->key + v->size, w->key + w->size);
                        std::move(v->rec, v->rec + v->size, w->rec + w->size);
                        w->size += v->size;
                        v->size = 0;

                        w->right = std::move(v->right);
                        if (not w->right.empty()) {
                            leaf_node *t = static_cast<leaf_node*>(std::malloc(sizeof(leaf_node)));
                            w->right.load(indexfile, *t);
                            t->left = std::move(v->left);
                            w->right.save(indexfile, *t);
                            std::free(t);
                        }

                        u.sub[loc].save(indexfile, *v);
                        leaf_node_pool.dealloc(u.sub[loc]);
                        std::move(u.key + loc,     u.key + u.size,     u.key + loc - 1);
                        std::move(u.sub + loc + 1, u.sub + u.size + 1, u.sub + loc    );
                        --u.size;
                    }
                    u.sub[loc - 1].save(indexfile, *w);
                    std::free(v); std::free(w);
                    return true;
                }
                if (loc < u.size) {
                    leaf_node *w = static_cast<leaf_node*>(std::malloc(sizeof(leaf_node)));
                    u.sub[loc + 1].load(indexfile, *w);

                    if (w->surplus()) {
                        v->key[v->size] = std::move(w->key[0]);
                        v->rec[v->size] = std::move(w->rec[0]);
                        ++v->size;
                        std::move(w->key + 1, w->key + w->size, w->key);
                        std::move(w->rec + 1, w->rec + w->size, w->rec);
                        --w->size;

                        u.key[loc] = v->key[v->size - 1];
                        u.sub[loc + 1].save(indexfile, *w);
                    } else {
                        std::move(w->key, w->key + w->size, v->key + v->size);
                        std::move(w->rec, w->rec + w->size, v->rec + v->size);
                        v->size += w->size;
                        w->size = 0;

                        v->right = std::move(w->right);
                        if (not v->right.empty()) {
                            leaf_node *t = static_cast<leaf_node*>(std::malloc(sizeof(leaf_node)));
                            v->right.load(indexfile, *t);
                            t->left = std::move(w->left);
                            v->right.save(indexfile, *t);
                            std::free(t);
                        }

                        u.sub[loc + 1].save(indexfile, *w);
                        leaf_node_pool.dealloc(u.sub[loc + 1]);
                        std::move(u.key + loc + 1, u.key + u.size,     u.key + loc    );
                        std::move(u.sub + loc + 2, u.sub + u.size + 1, u.sub + loc + 1);
                        --u.size;
                    }

                    u.sub[loc].save(indexfile, *v);
                    std::free(v); std::free(w);
                    return true;
                }
            }
            if (result) u.sub[loc].save(indexfile, *v);
            std::free(v);
        } else {
            internal_node *v = static_cast<internal_node*>(std::malloc(sizeof(internal_node)));
            u.sub[loc].load(indexfile, *v);
            bool result = erase(*v, key);

            if (v->scanty()) {
                if (0 < loc) {
                    internal_node *w = static_cast<internal_node*>(std::malloc(sizeof(internal_node)));
                    u.sub[loc - 1].load(indexfile, *w);

                    if (w->surplus()) {
                        /* get key from surplus brothers */
                        std::move_backward(v->key, v->key + v->size,     v->key + v->size + 1);
                        std::move_backward(v->sub, v->sub + v->size + 1, v->sub + v->size + 2);
                        ++v->size;
                        v->key[0] = std::move(u.key[loc - 1]);
                        u.key[loc - 1] = std::move(w->key[w->size - 1]);
                        v->sub[0] = std::move(w->sub[w->size]);
                        --w->size;

                        u.sub[loc].save(indexfile, *v);
                    } else {
                        /* merge with brothers */
                        w->key[w->size] = std::move(u.key[loc - 1]);
                        std::move(v->key, v->key + v->size,     w->key + w->size + 1);
                        std::move(v->sub, v->sub + v->size + 1, w->sub + w->size + 1);
                        w->size += v->size + 1;
                        v->size = 0;

                        internal_node_pool.dealloc(u.sub[loc]);
                        std::move(u.key + loc,     u.key + u.size,     u.key + loc - 1);
                        std::move(u.sub + loc + 1, u.sub + u.size + 1, u.sub + loc    );
                        --u.size;
                    }

                    u.sub[loc - 1].save(indexfile, *w);
                    std::free(v); std::free(w);
                    return true;
                }
                if (loc < u.size) {
                    internal_node *w = static_cast<internal_node*>(std::malloc(sizeof(internal_node)));
                    u.sub[loc + 1].load(indexfile, *w);

                    if (w->surplus()) {
                        v->key[v->size] = std::move(u.key[loc]);
                        u.key[loc] = std::move(w->key[0]);
                        v->sub[v->size + 1] = std::move(w->sub[0]);
                        ++v->size;
                        std::move(w->key + 1, w->key + w->size,     w->key);
                        std::move(w->sub + 1, w->sub + w->size + 1, w->sub);
                        --w->size;

                        u.sub[loc + 1].save(indexfile, *w);
                    } else {
                        v->key[v->size] = std::move(u.key[loc]);
                        std::move(w->key, w->key + w->size,     v->key + v->size + 1);
                        std::move(w->sub, w->sub + w->size + 1, v->sub + v->size + 1);
                        v->size += w->size + 1;
                        w->size = 0;

                        internal_node_pool.dealloc(u.sub[loc + 1]);
                        std::move(u.key + loc + 1, u.key + u.size,     u.key + loc    );
                        std::move(u.sub + loc + 2, u.sub + u.size + 1, u.sub + loc + 1);
                        --u.size;
                    }

                    u.sub[loc].save(indexfile, *v);
                    std::free(v); std::free(w);
                    return true;
                }
            }
            if (result) u.sub[loc].save(indexfile, *v);
            std::free(v);
        } return false;
    }

    template <typename Key, typename Value, typename Compare, i32 FACTOR>
    auto bptree<Key, Value, Compare, FACTOR>::find(internal_node &u, const key_type &key) -> iterator {
        size_type loc = std::upper_bound(u.key, u.key + u.size, key, key_le) - u.key;
        if (u.sub_is_leaf) {
            leaf_node *v = static_cast<leaf_node*>(std::malloc(sizeof(leaf_node)));
            u.sub[loc].load(indexfile, *v);
            auto tmp = find(*v, key);
            std::free(v);
            return tmp;
        } else {
            internal_node *v = static_cast<internal_node*>(std::malloc(sizeof(internal_node)));
            u.sub[loc].load(indexfile, *v);
            auto tmp = find(*v, key);
            std::free(v);
            return tmp;
        }
    }

    template <typename Key, typename Value, typename Compare, i32 FACTOR>
    auto bptree<Key, Value, Compare, FACTOR>::value(internal_node &u, const key_type &key) -> value_type {
        size_type loc = std::upper_bound(u.key, u.key + u.size, key, key_le) - u.key;
        if (u.sub_is_leaf) {
            leaf_node *v = static_cast<leaf_node*>(std::malloc(sizeof(leaf_node)));
            u.sub[loc].load(indexfile, *v);
            auto tmp = value(*v, key);
            std::free(v);
            return tmp;
        } else {
            internal_node *v = static_cast<internal_node*>(std::malloc(sizeof(internal_node)));
            u.sub[loc].load(indexfile, *v);
            auto tmp = value(*v, key);
            std::free(v);
            return tmp;
        }
    }

/* } */

/* impl btree<Key, Value, Compare, FACTOR> { */

    template <typename Key, typename Value, typename Compare, i32 FACTOR>
    bptree<Key, Value, Compare, FACTOR>::bptree(const str &filename_data, const str &filename_index) {
        datafile.open(filename_data);
        root = new internal_node;
        if (indexfile.open(filename_index)) {
            indexfile.read(header);
            header.root.load(indexfile, *root);
        } else {
            header.root = HardDisk::Record(sizeof(header));
            root->sub[0] = HardDisk::Record(sizeof(header) + sizeof(internal_node));
            root->sub_is_leaf = true;
            indexfile.write(header);
            header.root.save(indexfile, *root);
            root->sub[0].save(indexfile, leaf_node());
        }
    }

    template <typename Key, typename Value, typename Compare, i32 FACTOR>
    bptree<Key, Value, Compare, FACTOR>::~bptree() {
        delete root;
        datafile.close();
        indexfile.close();
    }

    template <typename Key, typename Value, typename Compare, i32 FACTOR>
    auto bptree<Key, Value, Compare, FACTOR>::insert(const key_type &key, const value_type &value) -> iterator {
        auto result = insert(*root, key, value);
        if (result.second) {
            header.root.save(indexfile, *root);
            root->size = 0;
            root->sub[0] = header.root;
            root->sub_is_leaf = false;
            header.root = internal_node_pool.alloc().save(indexfile, *root);
            indexfile.seek(0);
            indexfile.write(header);
        } return result.first;
    }

    template <typename Key, typename Value, typename Compare, i32 FACTOR>
    auto bptree<Key, Value, Compare, FACTOR>::erase(const key_type &key) -> void { erase(*root, key); }

    template <typename Key, typename Value, typename Compare, i32 FACTOR>
    auto bptree<Key, Value, Compare, FACTOR>::find(const key_type &key) -> iterator { return find(*root, key); }

    template <typename Key, typename Value, typename Compare, i32 FACTOR>
    auto bptree<Key, Value, Compare, FACTOR>::value(const key_type &key) -> value_type { return value(*root, key); }

/* } */

template <typename Key, typename Value, typename Compare, i32 FACTOR>
class bptree<Key, Value, Compare, FACTOR>::iterator {
    using Up    = bptree;
    using Self  = iterator;

    struct data_proxy;

    Up          *up;
    leaf_node   u;
    i32         loc;


public:
    iterator(): loc(-1) {}
    iterator(Up *__up, leaf_node node, i32 __loc): up(__up), u(node), loc(__loc) {}

    auto operator + (difference_type diff) -> Self;
    auto operator - (difference_type diff) -> Self;
    auto operator += (difference_type diff) -> Self& { return *this = *this + diff; }
    auto operator -= (difference_type diff) -> Self& { return *this = *this - diff; }

    auto operator ++ (int) -> Self { Self tmp = *this; *this = *this + 1; return tmp; }
    auto operator -- (int) -> Self { Self tmp = *this; *this = *this - 1; return tmp; }

    auto operator ++ () -> Self& { return *this = *this + 1; }
    auto operator -- () -> Self& { return *this = *this - 1; }

    // auto operator * () const -> value_type;
    auto operator * () const -> data_proxy;

    auto operator == (const Self &rhs) const -> bool {
        if (up != rhs.up or loc != rhs.loc) return false;
        return loc == -1 or std::memcmp(std::addressof(u), std::addressof(rhs.u), sizeof(u)) == 0;
    }
    auto operator != (const Self &rhs) const -> bool { return not (*this == rhs); }
};

/* impl bptree<Key, Value, Compare, FACTOR>::iterator { */

    template <typename Key, typename Value, typename Compare, i32 FACTOR>
    auto bptree<Key, Value, Compare, FACTOR>::iterator::operator + (difference_type diff) -> Self {
		if (diff < 0) return *this - (-diff);
        iterator dst(*this);
        for ( ; ; ) {
            difference_type rest = dst.u.size - dst.loc;
            if (diff < rest) return dst.loc += diff, dst;
            if (dst.u.right.empty()) return iterator();
            dst.u = dst.u.right.template get<leaf_node>(up->indexfile);
            dst.loc = 0;
            diff -= rest;
        }
    }

    template <typename Key, typename Value, typename Compare, i32 FACTOR>
    auto bptree<Key, Value, Compare, FACTOR>::iterator::operator - (difference_type diff) -> Self {
        if (diff < 0) return *this + (-diff);
        iterator dst(*this);
        for ( ; ; ) {
            difference_type rest = dst.loc;
            if (diff <= rest) return dst.loc -= diff, dst;
            if (dst.u.left.empty()) return iterator();
            dst.u = dst.u.left.template get<leaf_node>(up->indexfile);
            dst.loc = dst.u.size;
            diff -= rest;
        }
    }

    // template <typename Key, typename Value, typename Compare, i32 FACTOR>
    // auto bptree<Key, Value, Compare, FACTOR>::iterator::operator * () const -> value_type {
    //     if (loc < 0 or loc >= u.size) throw "dereference nullptr";
    //     return u.rec[loc].template get<value_type>(up->datafile);
    // }
    template <typename Key, typename Value, typename Compare, i32 FACTOR>
    auto bptree<Key, Value, Compare, FACTOR>::iterator::operator * () const -> data_proxy {
        if (loc < 0 or loc >= i32(u.size)) throw "dereference nullptr";
        return data_proxy(up->datafile, u.rec[loc]);
    }

/* } */

template <typename Key, typename Value, typename Compare, i32 FACTOR>
struct bptree<Key, Value, Compare, FACTOR>::iterator::data_proxy {
    HardDisk::IO &io;
    HardDisk::Record rec;
    value_type value;

    data_proxy(HardDisk::IO &__io, HardDisk::Record __rec): io(__io), rec(__rec) { rec.load(io, value); }
    ~data_proxy() { rec.save(io, value); }

    operator value_type&() { return value; }
    operator const value_type&() const { return value; }
};

}
