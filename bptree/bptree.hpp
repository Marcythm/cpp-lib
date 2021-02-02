#pragma once

#include "../config.hpp"
#include "HardDiskSupport/IO.hpp"
#include "HardDiskSupport/Record.hpp"

template <typename Key, typename Value, typename Compare = std::less<Key>>
class bptree {
    static constexpr i32 FACTOR = 100;

    using Self          = bptree;

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

    key_compare key_leq;
    auto key_eq(const key_type &lhs, const key_type &rhs) const -> bool { return key_leq(lhs, rhs) and key_leq(rhs, lhs); }

    HardDisk::IO datafile, indexfile;
    HardDisk::RecordPool<value_type> datapool;
    HardDisk::RecordPool<leaf_node> leaf_node_pool;
    HardDisk::RecordPool<internal_node> internal_node_pool;
    internal_node root;

public:
    bptree(const str & = str("data.bin"), const str & = str("index.bin"));

private:
    auto insert(leaf_node &u, const key_type &key, const value_type &value) -> bool;
    auto erase(leaf_node &u, const key_type &key) -> bool;
    auto find(leaf_node &u, const key_type &key) -> value_type;

    auto insert(internal_node &u, const key_type &key, const value_type &value) -> bool;
    auto erase(internal_node &u, const key_type &key) -> bool;
    auto find(internal_node &u, const key_type &key) -> value_type;

public:
    auto insert(const key_type &key, const value_type &value) -> void;
    auto erase(const key_type &key) -> void;
    auto find(const key_type &key) -> value_type;
};


template <typename Key, typename Value, typename Compare>
struct bptree<Key, Value, Compare>::leaf_node {
    static constexpr i32 MIN_KEY_NUM = (FACTOR - 1) / 2 - 1;
    static constexpr i32 MAX_KEY_NUM = FACTOR - 1;
    static constexpr i32 MAX_REC_NUM = MAX_KEY_NUM;

    size_type           size;
    HardDisk::Record    left, right;
    key_type            key[MAX_KEY_NUM + 1];
    HardDisk::Record    rec[MAX_REC_NUM + 1];

    auto scanty()  const -> bool { return size < MIN_KEY_NUM; }
    auto surplus() const -> bool { return size > MIN_KEY_NUM; }
    auto full()    const -> bool { return size > MAX_KEY_NUM; }

    leaf_node(): size(0) {}
};

/* impl bptree<Key, Value, Compare>::leaf_node { */

    template <typename Key, typename Value, typename Compare>
    auto bptree<Key, Value, Compare>::insert(leaf_node &u, const key_type &key, const value_type &value) -> bool {
        size_type loc = std::lower_bound(u.key, u.key + u.size, key, key_leq) - u.key;
        if (loc < u.size and key_eq(key, u.key[loc]))
            u.rec[loc].save(datafile, value);
        else {
            std::move_backward(u.key + loc, u.key + u.size, u.key + u.size + 1);
            std::move_backward(u.rec + loc, u.rec + u.size, u.rec + u.size + 1);
            u.key[loc] = key;
            u.rec[loc].save(datafile, value);
            ++u.size;
            return true;
        } return false;
    }

    template <typename Key, typename Value, typename Compare>
    auto bptree<Key, Value, Compare>::erase(leaf_node &u, const key_type &key) -> bool {
        size_type loc = std::lower_bound(u.key, u.key + u.size, key, key_leq) - u.key;
        if (loc < u.size and key_eq(key, u.key[loc])) {
            datapool.dealloc(u.rec[loc]);
            std::move(u.key + loc + 1, u.key + u.size, u.key + loc);
            std::move(u.rec + loc + 1, u.rec + u.size, u.rec + loc);
            --u.size;
            return true;
        } return false;
    }

    template <typename Key, typename Value, typename Compare>
    auto bptree<Key, Value, Compare>::find(leaf_node &u, const key_type &key) -> value_type {
        size_type loc = std::lower_bound(u.key, u.ley + u.size, key, key_leq) - u.key;
        if (loc < u.size and key_eq(key, u.key[loc]))
            return u.rec[loc].template get<value_type>();
        // return value_type();
    }

/* } */


template <typename Key, typename Value, typename Compare>
struct bptree<Key, Value, Compare>::internal_node {
    static constexpr i32 MIN_KEY_NUM = (FACTOR - 1) / 2 - 1;
    static constexpr i32 MAX_KEY_NUM = FACTOR - 1;
    static constexpr i32 MAX_SUB_NUM = MAX_KEY_NUM + 1;

    bool                sub_is_leaf;
    size_type           size;
    key_type            key[MAX_KEY_NUM + 1];
    HardDisk::Record    sub[MAX_SUB_NUM + 1];

    auto scanty()  const -> bool { return size < MIN_KEY_NUM; }
    auto surplus() const -> bool { return size > MIN_KEY_NUM; }
    auto full()    const -> bool { return size > MAX_KEY_NUM; }

    internal_node(): sub_is_leaf(false), size(0) {}
};

/* impl bptree<Key, Value, Compare>::internal_node { */

    template <typename Key, typename Value, typename Compare>
    auto bptree<Key, Value, Compare>::insert(internal_node &u, const key_type &key, const value_type &value) -> bool {
        size_type loc = std::upper_bound(u.key, u.key + u.size, key, key_leq) - u.key;
        if (u.sub_is_leaf) {
            leaf_node v;
            u.sub[loc].load(indexfile, v);
            bool modified = insert(v, key, value);

            /* if full then split */
            if (v.full()) {
                leaf_node w;
                std::move(v.key + FACTOR / 2, v.key + v.size, w.key);
                std::move(v.rec + FACTOR / 2, v.rec + v.size, w.rec);
                w.size = v.size - FACTOR / 2;
                v.size = FACTOR / 2;

                std::move_backward(u.key + loc,     u.key + u.size,     u.key + u.size + 1);
                std::move_backward(u.sub + loc + 1, u.sub + u.size + 1, u.sub + u.size + 2);
                u.key[loc] = w.key[0];
                ++u.size;

                u.sub[loc].save(indexfile, v);
                u.sub[loc + 1] = leaf_node_pool.alloc().save(indexfile, w);

                w.left = u.sub[loc];
                w.right = std::move(v.right);
                v.right = u.sub[loc + 1];
                if (not w.right.empty()) {
                    leaf_node t;
                    w.right.load(indexfile, t);
                    t.left = v.right;
                    w.right.save(indexfile, t);
                }

                return true;
            }
            if (modified)
                u.sub[loc].save(indexfile, v);
        } else {
            internal_node v;
            u.sub[loc].load(indexfile, v);
            bool modified = insert(v, key, value);

            /* if full then split */
            if (v.full()) {
                internal_node w;
                std::move(v.key + FACTOR / 2 + 1, v.key + v.size,     w.key);
                std::move(v.sub + FACTOR / 2,     v.sub + v.size + 1, w.sub);
                w.size = v.size - FACTOR / 2 - 1;
                v.size = FACTOR / 2;
                w.sub_is_leaf = v.sub_is_leaf;

                std::move_backward(u.key + loc,     u.key + u.size, u.key + u.size + 1);
                std::move_backward(u.rec + loc + 1, u.rec + u.size, u.rec + u.size + 1);
                u.key[loc] = std::move(v.key[FACTOR / 2]);
                ++u.size;

                u.sub[loc].save(indexfile, v);
                u.sub[loc + 1] = internal_node_pool.alloc().save(index, w);
                return true;
            }
            if (modified)
                u.sub[loc].save(indexfile, v);
        } return false;
    }

    template <typename Key, typename Value, typename Compare>
    auto bptree<Key, Value, Compare>::erase(internal_node &u, const key_type &key) -> bool {
        size_type loc = std::upper_bound(u.key, u.key + u.size, key, key_leq) - u.key;
        if (u.sub_is_leaf) {
            leaf_node v;
            u.sub[loc].load(indexfile, v);
            bool modified = erase(v, key);

            if (v.scanty()) {
                /* try to get key from surplus brothers */
                if (0 < loc) {
                    leaf_node w;
                    u.sub[loc - 1].load(indexfile, w);

                    if (w.surplus()) {
                        std::move_backward(v.key, v.key + v.size, v.key + v.size + 1);
                        std::move_backward(v.rec, v.rec + v.size, v.rec + v.size + 1);
                        --w.size;
                        v.key[0] = std::move(w.key[w.size]);
                        v.rec[0] = std::move(w.rec[w.size]);
                        ++v.size;

                        u.key[loc - 1] = v.key[0];

                        u.sub[loc - 1].save(indexfile, w);
                        u.sub[loc].save(indexfile, v);
                        return true;
                    }
                }
                if (loc < u.size) {
                    leaf_node w;
                    u.sub[loc + 1].load(indexfile, w);

                    if (w.surplus()) {
                        v.key[v.size] = std::move(w.key[0]);
                        v.rec[v.size] = std::move(w.rec[0]);
                        ++v.size;
                        std::move(w.key + 1, w.key + w.size, w.key);
                        std::move(w.rec + 1, w.rec + w.size, w.rec);
                        --w.size;

                        u.key[loc] = v.key[v.size - 1];

                        u.sub[loc].save(indexfile, v);
                        u.sub[loc + 1].save(indexfile, w);
                        return true;
                    }
                }

                /* merge with brothers */
                if (0 < loc) {
                    leaf_node w;
                    u.sub[loc - 1].load(indexfile, w);

                    std::move(v.key, v.key + v.size, w.key + w.size);
                    std::move(v.rec, v.rec + v.size, w.rec + w.size);
                    w.size += v.size;
                    v.size = 0;

                    w.right = std::move(v.right);
                    if (not w.right.empty()) {
                        leaf_node t;
                        w.right.load(indexfile, t);
                        t.left = v.left;
                        w.right.save(indexfile, t);
                    }

                    leaf_node_pool.dealloc(u.sub[loc]);
                    std::move(u.key + loc,     u.key + u.size,     u.key + loc - 1);
                    std::move(u.sub + loc + 1, u.sub + u.size + 1, u.sub + loc    );
                    --u.size;

                    u.sub[loc - 1].save(indexfile, w);
                    return true;
                }
                if (loc < u.size) {
                    leaf_node w;
                    u.sub[loc + 1].load(indexfile, w);

                    std::move(w.key, w.key + w.size, v.key + v.size);
                    std::move(w.rec, w.rec + w.size, v.rec + v.size);
                    v.size += w.size;
                    w.size = 0;

                    v.right = std::move(w.right);
                    if (not v.right.empty()) {
                        leaf_node t;
                        v.right.load(indexfile, t);
                        t.left = w.left;
                        v.right.save(indexfile, t);
                    }

                    leaf_node_pool.dealloc(u.sub[loc + 1]);
                    std::move(u.key + loc + 1, u.key + u.size,     u.key + loc    );
                    std::move(u.sub + loc + 2, u.sub + u.size + 1, u.sub + loc + 1);
                    --u.size;

                    u.sub[loc].save(indexfile, v);
                    return true;
                }
            }
            if (modified)
                u.sub[loc].save(indexfile, v);
        } else {
            internal_node v;
            u.sub[loc].load(indexfile, v);
            bool modified = erase(v, key);

            if (v.scanty()) {
                /* try to get key from surplus brothers */
                if (0 < loc) {
                    internal_node w;
                    u.sub[loc - 1].load(indexfile, w);

                    if (w.surplus()) {
                        std::move_backward(v.key, v.key + v.size,     v.key + v.size + 1);
                        std::move_backward(v.sub, v.sub + v.size + 1, v.sub + v.size + 2);
                        ++v.size;
                        v.key[0] = std::move(u.key[loc - 1]);
                        u.key[loc - 1] = std::move(w.key[w.size - 1]);
                        v.sub[0] = std::move(w.sub[w.size]);
                        --w.size;

                        u.sub[loc - 1].save(indexfile, w);
                        u.sub[loc].save(indexfile, v);
                        return true;
                    }
                }
                if (loc < u.size) {
                    internal_node w;
                    u.sub[loc + 1].load(indexfile, w);

                    if (w.surplus()) {
                        v.key[v.size] = std::move(u.key[loc]);
                        u.key[loc] = std::move(w.key[0]);
                        v.sub[v.size + 1] = std::move(w.sub[0]);
                        ++v.size;
                        std::move(w.key + 1, w.key + w.size,     w.key);
                        std::move(w.sub + 1, w.sub + w.size + 1, w.sub);
                        --w.size;

                        u.sub[loc].save(indexfile, v);
                        u.sub[loc + 1].save(indexfile, w);
                        return true;
                    }
                }

                /* merge with brothers */
                if (0 < loc) {
                    internal_node w;
                    u.sub[loc - 1].load(indexfile, w);

                    w.key[w.size] = std::move(u.key[loc - 1]);
                    std::move(v.key, v.key + v.size,     w.key + w.size + 1);
                    std::move(v.sub, v.sub + v.size + 1, w.sub + w.size + 1);
                    w.size += v.size + 1;
                    v.size = 0;

                    internal_node_pool.dealloc(u.sub[loc]);
                    std::move(u.key + loc,     u.key + u.size,     u.key + loc - 1);
                    std::move(u.sub + loc + 1, u.sub + u.size + 1, u.sub + loc    );
                    --u.size;

                    u.sub[loc - 1].save(indexfile, w);
                    return true;
                }
                if (loc < u.size) {
                    internal_node w;
                    u.sub[loc + 1].load(indexfile, w);

                    v.key[v.size] = std::move(u.key[loc]);
                    std::move(w.key, w.key + w.size,     v.key + v.size + 1);
                    std::move(w.sub, w.sub + w.size + 1, v.sub + v.size + 1);
                    v.size += w.size + 1;
                    w.size = 0;

                    internal_node_pool.dealloc(u.sub[loc + 1]);
                    std::move(u.key + loc + 1, u.key + u.size,     u.key + loc    );
                    std::move(u.sub + loc + 2, u.sub + u.size + 1, u.sub + loc + 1);
                    --u.size;

                    u.sub[loc].save(indexfile, v);
                    return true;
                }
            }
            if (modified)
                u.sub[loc].save(indexfile, v);
        } return false;
    }

    template <typename Key, typename Value, typename Compare>
    auto bptree<Key, Value, Compare>::find(internal_node &u, const key_type &key) -> value_type {
        size_type loc = std::upper_bound(u.key, u.ley + u.size, key, key_leq) - u.key;
        if (u.sub_is_leaf) {
            leaf_node v;
            u.sub[loc].load(indexfile, v);
            return find(v, key);
        } else {
            internal_node v;
            u.sub[loc].load(indexfile, v);
            return find(v, key);
        }
    }

/* } */

/* impl btree<Key, Value, Compare> { */

    template <typename Key, typename Value, typename Compare>
    bptree<Key, Value, Compare>::bptree(const str &filename_data, const str &filename_index) {
        datafile.open(filename_data);
        if (indexfile.open(filename_index)) {
            indexfile.read(header);
            header.root.load(indexfile, root);
        } else {
            header.root = HardDisk::Record(sizeof(header));
            root.sub[0] = HardDisk::Record(sizeof(header) + sizeof(internal_node));
            root.sub_is_leaf = true;
            indexfile.write(header);
            header.root.save(indexfile, root);
            root.sub[0].save(indexfile, leaf_node());
        }
    }

    template <typename Key, typename Value, typename Compare>
    auto bptree<Key, Value, Compare>::insert(const key_type &key, const value_type &value) -> void {
        insert(root, key, value);
        if (root.size > 0) {
            internal_node newroot;
            newroot.sub[0] = header.root;
            header.root = internal_node_pool.alloc().save(indexfile, newroot);
            indexfile.seek(0);
            indexfile.write(header);
            root = newroot;
        }
    }

    template <typename Key, typename Value, typename Compare>
    auto bptree<Key, Value, Compare>::erase(const key_type &key) -> void { erase(root, key); }

    template <typename Key, typename Value, typename Compare>
    auto bptree<Key, Value, Compare>::find(const key_type &key) -> value_type { return find(root, key); }

/* } */

template <typename Key, typename Value, typename Compare>
class bptree<Key, Value, Compare>::iterator {
    using Up    = bptree;
    using Self  = iterator;

    struct data_proxy;

    Up          *up;
    leaf_node   u;
    i32         loc;


public:
    iterator(): loc(-1) {}
    iterator(Up *__up, leaf_node node, i32 __loc): up(__up), u(node), loc(__loc) {}
    iterator(const Self &other): up(other.up), u(other.u), loc(other.loc) {}

    auto operator + (difference_type diff) -> Self;
    auto operator - (difference_type diff) -> Self;
    auto operator += (difference_type diff) -> Self& { return *this = *this + diff; }
    auto operator -= (difference_type diff) -> Self& { return *this = *this - diff; }

    auto operator ++ (int) -> Self { Self tmp = *this; *this = *this + 1; return tmp; }
    auto operator -- (int) -> Self { Self tmp = *this; *this = *this - 1; return tmp; }

    auto operator ++ () -> Self& { return *this = *this + 1; }
    auto operator -- () -> Self& { return *this = *this - 1; }

    auto operator * () const -> data_proxy;

    auto operator == (const Self &rhs) const -> bool {
        return up == rhs.up and loc == rhs.loc and std::memcmp(std::addressof(u), std::addressof(rhs.u), sizeof(u)) == 0;
    }
    auto operator != (const Self &rhs) const -> bool { return not (*this == rhs); }
};

/* impl bptree<Key, Value, Compare>::iterator { */

    template <typename Key, typename Value, typename Compare>
    auto bptree<Key, Value, Compare>::iterator::operator + (difference_type diff) -> Self {
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

    template <typename Key, typename Value, typename Compare>
    auto bptree<Key, Value, Compare>::iterator::operator - (difference_type diff) -> Self {
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

    template <typename Key, typename Value, typename Compare>
    auto bptree<Key, Value, Compare>::iterator::operator * () const -> data_proxy {
        return data_proxy(u.rec[loc], up->datafile);
    }

/* } */

template <typename Key, typename Value, typename Compare>
struct bptree<Key, Value, Compare>::iterator::data_proxy {
    HardDisk::IO &io;
    HardDisk::Record rec;
    value_type value;

    data_proxy(HardDisk::Record __rec, HardDisk::IO &__io): rec(__rec), io(__io) { rec.load(io, value); }
    ~data_proxy() { rec.save(io, value); }

    operator value_type&() { return value; }
    operator const value_type&() { return value; }
};
