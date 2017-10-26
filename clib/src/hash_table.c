/*
 * hash.c
 *
 *  Created on: 2012-5-29
 *      Author: Administrator
 */

#include <clib/hash_table.h>
//#include <clib/memcache.h>
#include <clib/memory.h>
#include <stdlib.h>
#include <clib/log.h>
#include <clib/string.h>
#include <strings.h>
#include <clib/socket.h>

#define HASH_TABLE_MIN_SHIFT 3  /* 1 << 3 == 8 buckets */

#define UNUSED_HASH_VALUE 0
#define TOMBSTONE_HASH_VALUE 1
#define HASH_IS_UNUSED(h_) ((h_) == UNUSED_HASH_VALUE)
#define HASH_IS_TOMBSTONE(h_) ((h_) == TOMBSTONE_HASH_VALUE)
#define HASH_IS_REAL(h_) ((h_) >= 2)

struct _hash_table {
    int size;
    int mod;
    uint mask;
    int nnodes;
    int noccupied; /* nnodes + tombstones */
    void **keys;
    uint *hashes;
    void **values;
    hash_func hash_func;
    compare_func key_equal_func;
    void *user_data;
    destroy_func key_destroy_func;
    destroy_func value_destroy_func;
};

struct _hash_table_iter {
    /*< private >*/
    void *dummy1;
    void *dummy2;
    void *dummy3;
    int dummy4;
    bool dummy5;
    void *dummy6;
};

static const int prime_mod[] = {  1, /* For 1 << 0 */
2, 3, 7, 13, 31, 61, 127, 251, 509, 1021, 2039, 4093, 8191, 16381, 32749, 65521, /* For 1 << 16 */
131071, 262139, 524287, 1048573, 2097143, 4194301, 8388593, 16777213, 33554393,
        67108859, 134217689, 268435399, 536870909, 1073741789, 2147483647 /* For 1 << 31 */
};

static void hash_table_set_shift(hash_table_t *hash_table, int shift)
{
    int i;
    uint mask = 0;

    hash_table->size = 1 << shift;
    hash_table->mod = prime_mod[shift];

    for (i = 0; i < shift; i++) {
        mask <<= 1;
        mask |= 1;
    }

    hash_table->mask = mask;
}

static int hash_table_find_closest_shift(int n)
{
    int i;

    for (i = 0; n; i++)
        n >>= 1;

    return i;
}

static void hash_table_set_shift_from_size(hash_table_t *hash_table, int size)
{
    int shift;

    shift = hash_table_find_closest_shift(size);
    shift = MAX (shift, HASH_TABLE_MIN_SHIFT);

    hash_table_set_shift(hash_table, shift);
}

hash_table_t* hash_table_new(/*uint size,*/hash_func hash_func,
        compare_func key_equal_func, void *user_data,
        destroy_func key_destroy_func, destroy_func value_destroy_func)
{
    hash_table_t *hash_table;

    if (hash_func == NULL || key_equal_func == NULL) {
        LOG(LOG_LEVEL_ERROR, "哈希函数和比较函数不能为空");
        return NULL;
    }

    hash_table = (hash_table_t*) mem_alloc(sizeof(hash_table_t));
    hash_table_set_shift(hash_table, HASH_TABLE_MIN_SHIFT);
    hash_table->nnodes = 0;
    hash_table->noccupied = 0;
    hash_table->hash_func = hash_func;
    hash_table->key_equal_func = key_equal_func;
    hash_table->key_destroy_func = key_destroy_func;
    hash_table->value_destroy_func = value_destroy_func;
    hash_table->keys = (void**) mem_alloc(sizeof(void*)*hash_table->size);
    bzero(hash_table->keys, sizeof(void*) * hash_table->size);
    hash_table->values = hash_table->keys;
    hash_table->hashes = (uint*) mem_alloc(sizeof(uint)*hash_table->size);
    bzero(hash_table->hashes, sizeof(uint) * hash_table->size);
    return hash_table;
}

static void hash_table_resize_internal(hash_table_t *hash_table)
{
    void **new_keys;
    void **new_values;
    uint *new_hashes;
    int old_size;
    int i;

    old_size = hash_table->size;
    hash_table_set_shift_from_size(hash_table, hash_table->nnodes * 2);

    new_keys = (void**) mem_alloc(sizeof(void*)* hash_table->size);
    bzero(new_keys, sizeof(void*) * hash_table->size);
    if (hash_table->keys == hash_table->values) {
        new_values = new_keys;
    } else {
        new_values = (void**) mem_alloc(sizeof(void*)* hash_table->size);
        bzero(new_values, sizeof(void*) * hash_table->size);
    }
    new_hashes = (uint*) mem_alloc(sizeof(uint)* hash_table->size);
    bzero(new_hashes, sizeof(uint) * hash_table->size);

    for (i = 0; i < old_size; i++) {
        uint node_hash = hash_table->hashes[i];
        uint hash_val;
        uint step = 0;

        if (!HASH_IS_REAL (node_hash))
            continue;

        hash_val = node_hash % hash_table->mod;

        while (!HASH_IS_UNUSED (new_hashes[hash_val])) {
            step++;
            hash_val += step;
            hash_val &= hash_table->mask;
        }

        new_hashes[hash_val] = hash_table->hashes[i];
        new_keys[hash_val] = hash_table->keys[i];
        new_values[hash_val] = hash_table->values[i];
    }

    if (hash_table->keys != hash_table->values)
        mem_free(hash_table->values);

    mem_free(hash_table->keys);
    mem_free(hash_table->hashes);

    hash_table->keys = new_keys;
    hash_table->values = new_values;
    hash_table->hashes = new_hashes;

    hash_table->noccupied = hash_table->nnodes;
}

static inline void hash_table_maybe_resize(hash_table_t *hash_table)
{
    int noccupied = hash_table->noccupied;
    int size = hash_table->size;

    if ((size > hash_table->nnodes * 4 && size > 1 << HASH_TABLE_MIN_SHIFT)
            || (size <= noccupied + (noccupied / 16)))
        hash_table_resize_internal(hash_table);
}

static void hash_table_remove_all_nodes(hash_table_t *hash_table, bool notify)
{
    int i;
    void *key;
    void *value;

    hash_table->nnodes = 0;
    hash_table->noccupied = 0;

    if (!notify
            || (hash_table->key_destroy_func == NULL
                    && hash_table->value_destroy_func == NULL)) {
        bzero(hash_table->hashes, hash_table->size * sizeof(uint));
        bzero(hash_table->keys, hash_table->size * sizeof(void*));
        bzero(hash_table->values, hash_table->size * sizeof(void*));

        return;
    }

    for (i = 0; i < hash_table->size; i++) {
        if (HASH_IS_REAL (hash_table->hashes[i])) {
            key = hash_table->keys[i];
            value = hash_table->values[i];

            hash_table->hashes[i] = UNUSED_HASH_VALUE;
            hash_table->keys[i] = NULL;
            hash_table->values[i] = NULL;

            if (hash_table->key_destroy_func != NULL)
                hash_table->key_destroy_func(key);

            if (hash_table->value_destroy_func != NULL)
                hash_table->value_destroy_func(value);
        } else if (HASH_IS_TOMBSTONE (hash_table->hashes[i])) {
            hash_table->hashes[i] = UNUSED_HASH_VALUE;
        }
    }
}

static void hash_table_remove_node(hash_table_t *hash_table, int i, bool notify)
{
    void *key;
    void *value;

    key = hash_table->keys[i];
    value = hash_table->values[i];

    /* Erect tombstone */
    hash_table->hashes[i] = TOMBSTONE_HASH_VALUE;

    /* Be GC friendly */
    hash_table->keys[i] = NULL;
    hash_table->values[i] = NULL;

    hash_table->nnodes--;

    if (notify && hash_table->key_destroy_func)
        hash_table->key_destroy_func(key);

    if (notify && hash_table->value_destroy_func)
        hash_table->value_destroy_func(value);

}

void hash_table_remove_all(hash_table_t *hash_table)
{
    RETURN_IF_FAIL(hash_table != NULL);

    hash_table_remove_all_nodes(hash_table, true);
    hash_table_maybe_resize(hash_table);
}

void hash_table_free(hash_table_t *hash_table)
{
    RETURN_IF_FAIL(hash_table != NULL);

    hash_table_remove_all_nodes(hash_table, true);
    if (hash_table->keys != hash_table->values)
        mem_free(hash_table->values);
    mem_free(hash_table->keys);
    mem_free(hash_table->hashes);
    mem_free(hash_table);
}

static inline uint hash_table_lookup_node(hash_table_t *hash_table,
        const void *key, uint *hash_return)
{
    uint node_index;
    uint node_hash;
    uint hash_value;
    uint first_tombstone = 0;
    bool have_tombstone = false;
    uint step = 0;

    hash_value = hash_table->hash_func(key);
    if (!HASH_IS_REAL (hash_value))
        hash_value = 2;

    *hash_return = hash_value;

    node_index = hash_value % hash_table->mod;
    node_hash = hash_table->hashes[node_index];

    while (!HASH_IS_UNUSED (node_hash)) {
        /* We first check if our full hash values
         * are equal so we can avoid calling the full-blown
         * key equality function in most cases.
         */
        if (node_hash == hash_value) {
            void *node_key = hash_table->keys[node_index];

            if (hash_table->key_equal_func) {
                if (hash_table->key_equal_func(node_key, key,
                        hash_table->user_data) == 0)
                    return node_index;
            } else if (node_key == key) {
                return node_index;
            }
        } else if (HASH_IS_TOMBSTONE (node_hash) && !have_tombstone) {
            first_tombstone = node_index;
            have_tombstone = true;
        }

        step++;
        node_index += step;
        node_index &= hash_table->mask;
        node_hash = hash_table->hashes[node_index];
    }

    if (have_tombstone)
        return first_tombstone;

    return node_index;
}

static void hash_table_insert_node(hash_table_t *hash_table, uint node_index,
        uint key_hash, void *key, void *value, bool keep_new_key,
        bool reusing_key)
{
    uint old_hash;
    void *old_key;
    void *old_value;

    if (hash_table->keys == hash_table->values && key != value)
        hash_table->values = memdup(hash_table->keys,
                sizeof(void *) * hash_table->size);

    old_hash = hash_table->hashes[node_index];
    old_key = hash_table->keys[node_index];
    old_value = hash_table->values[node_index];

    if (HASH_IS_REAL (old_hash)) {
        if (keep_new_key)
            hash_table->keys[node_index] = key;
        hash_table->values[node_index] = value;
    } else {
        hash_table->keys[node_index] = key;
        hash_table->values[node_index] = value;
        hash_table->hashes[node_index] = key_hash;

        hash_table->nnodes++;

        if (HASH_IS_UNUSED (old_hash)) {
            /* We replaced an empty node, and not a tombstone */
            hash_table->noccupied++;
            hash_table_maybe_resize(hash_table);
        }

    }

    if (HASH_IS_REAL (old_hash)) {
        if (hash_table->key_destroy_func && !reusing_key)
            hash_table->key_destroy_func(keep_new_key ? old_key : key);
        if (hash_table->value_destroy_func)
            hash_table->value_destroy_func(old_value);
    }
}

static void hash_table_insert_internal(hash_table_t *hash_table, void *key,
        void *value, bool keep_new_key)
{
    uint key_hash;
    uint node_index;

    RETURN_IF_FAIL(hash_table != NULL);

    node_index = hash_table_lookup_node(hash_table, key, &key_hash);

    hash_table_insert_node(hash_table, node_index, key_hash, key, value,
            keep_new_key, false);
}

void hash_table_insert(hash_table_t *hash_table, void *key, void *value)
{
    RETURN_IF_FAIL(hash_table != NULL);
    RETURN_IF_FAIL(key != NULL);

    hash_table_insert_internal(hash_table, key, value, false);
}

void hash_table_replace(hash_table_t *hash_table, void *key, void *value)
{
    RETURN_IF_FAIL(hash_table != NULL);
    RETURN_IF_FAIL(key != NULL);

    hash_table_insert_internal(hash_table, key, value, true);
}

static bool hash_table_remove_internal(hash_table_t *hash_table,
        const void * key, bool notify, void **orikey, void **value)
{
    uint node_index;
    uint node_hash;

    RETURN_VAL_IF_FAIL(hash_table != NULL, false);

    node_index = hash_table_lookup_node(hash_table, key, &node_hash);

    if (!HASH_IS_REAL (hash_table->hashes[node_index]))
        return false;

    if (!notify) { //steal
        if (orikey != NULL) {
            *orikey = hash_table->keys[node_index];
        }
        if (value != NULL) {
            *value = hash_table->values[node_index];
        }
    }

    hash_table_remove_node(hash_table, node_index, notify);
//    hash_table_maybe_resize(hash_table);

    return true;
}

void hash_table_resize(hash_table_t *hash_table)
{
    RETURN_IF_FAIL(hash_table != NULL);

    hash_table_maybe_resize(hash_table);
}

bool hash_table_remove(hash_table_t *hash_table, const void * key)
{
    RETURN_VAL_IF_FAIL(hash_table != NULL, false);

    return hash_table_remove_internal(hash_table, key, true, NULL, NULL);
}

bool hash_table_steal(hash_table_t *hash_table, const void * key, void **orikey,
        void **value)
{
    RETURN_VAL_IF_FAIL(hash_table != NULL, false);
    RETURN_VAL_IF_FAIL(key != NULL, false);

    return hash_table_remove_internal(hash_table, key, false, orikey, value);
}

bool hash_table_contains(hash_table_t *hash_table, const void *key)
{

    uint node_index;
    uint node_hash;

    RETURN_VAL_IF_FAIL(hash_table != NULL, false);
    RETURN_VAL_IF_FAIL(key != NULL, false);

    node_index = hash_table_lookup_node(hash_table, key, &node_hash);

    return HASH_IS_REAL (hash_table->hashes[node_index]);

}

void *hash_table_lookup(hash_table_t *hash_table, const void *key)
{
    uint node_index;
    uint node_hash;

    RETURN_VAL_IF_FAIL(hash_table != NULL, NULL);
    RETURN_VAL_IF_FAIL(key != NULL, false);

    node_index = hash_table_lookup_node(hash_table, key, &node_hash);

    return HASH_IS_REAL (hash_table->hashes[node_index]) ?
            hash_table->values[node_index] : NULL;
}

bool hash_table_lookup_extended(hash_table_t *hash_table,
        const void *lookup_key, void **orig_key, void **value)
{
    uint node_index;
    uint node_hash;

    RETURN_VAL_IF_FAIL(hash_table != NULL, false);
    RETURN_VAL_IF_FAIL(lookup_key != NULL, false);

    node_index = hash_table_lookup_node(hash_table, lookup_key, &node_hash);

    if (!HASH_IS_REAL (hash_table->hashes[node_index]))
        return false;

    if (orig_key)
        *orig_key = hash_table->keys[node_index];

    if (value)
        *value = hash_table->values[node_index];

    return true;
}

bool hash_table_foreach(hash_table_t *hash_table, traverse_pair_func func,
        void *user_data)
{
    int i;

    RETURN_VAL_IF_FAIL(hash_table != NULL, false);
    RETURN_VAL_IF_FAIL(func != NULL, false);

    for (i = 0; i < hash_table->size; i++) {
        uint node_hash = hash_table->hashes[i];
        void *node_key = hash_table->keys[i];
        void *node_value = hash_table->values[i];

        if (HASH_IS_REAL (node_hash)) {
            if ((*func)(node_key, node_value, user_data)) {
                return false;
            }
        }
    }
    return true;
}

static uint hash_table_foreach_remove_or_steal(hash_table_t *hash_table,
        traverse_pair_func func, void *user_data, bool notify)
{
    uint deleted = 0;
    int i;

    for (i = 0; i < hash_table->size; i++) {
        uint node_hash = hash_table->hashes[i];
        void *node_key = hash_table->keys[i];
        void *node_value = hash_table->values[i];

        if (HASH_IS_REAL (node_hash)
                && (*func)(node_key, node_value, user_data)) {
            hash_table_remove_node(hash_table, i, notify);
            deleted++;
        }

    }

//    hash_table_maybe_resize(hash_table);

    return deleted;
}

uint hash_table_foreach_remove(hash_table_t *hash_table,
        traverse_pair_func func, void *user_data)
{
    RETURN_VAL_IF_FAIL(hash_table != NULL, 0);
    RETURN_VAL_IF_FAIL(func != NULL, 0);

    return hash_table_foreach_remove_or_steal(hash_table, func, user_data, true);
}

uint hash_table_size(hash_table_t *hash_table)
{
    RETURN_VAL_IF_FAIL(hash_table != NULL, 0);

    return hash_table->nnodes;
}

//list_t* hash_table_get_keys(hash_table_t *hash_table)
//{
//    int i;
//    list_t *retval;
//
//    RETURN_VAL_IF_FAIL(hash_table != NULL, NULL);
//
//    retval = list_new(NULL);
//    for (i = 0; i < hash_table->size; i++) {
//        if (HASH_IS_REAL (hash_table->hashes[i]))
//            list_prepend(retval, hash_table->keys[i]);
//    }
//
//    return retval;
//}
//
//list_t* hash_table_get_values(hash_table_t *hash_table)
//{
//    int i;
//    list_t *retval;
//
//    RETURN_VAL_IF_FAIL(hash_table != NULL, NULL);
//
//    retval = list_new(NULL);
//    for (i = 0; i < hash_table->size; i++) {
//        if (HASH_IS_REAL (hash_table->hashes[i]))
//            list_prepend(retval, hash_table->values[i]);
//    }
//
//    return retval;
//}

uint str_hash(const void *v)
{
    const signed char *p;
    uint32_t h = 5381;

    for (p = v; *p != '\0'; p++)
        h = (h << 5) + h + *p;

    return h;
}

uint int32_hash(const void *v)
{
    return *(const int32_t*) v;
}

uint int_hash(const void *v)
{
    return *(const int*) v;
}

uint int64_hash(const void *v)
{
    return (uint) *(const int64_t*) v;
}

uint double_hash(const void *v)
{
    return (uint) *(const double*) v;
}

uint direct_hash(const void *v)
{
    return (uint) v;
}

uint ip_hash(const void *v)
{
    struct ip_addr *ip = (struct ip_addr *) v;
    if (ip->version == IPV4) {
        return ip->addr.ipv4;
    } else { //IPV6
        LOG(LOG_LEVEL_ERROR, "IPV6地址hash函数实现");
        return 0;
    }
}

void hash_table_destroy_func(void* data)
{
    hash_table_free((hash_table_t*) data);
}
