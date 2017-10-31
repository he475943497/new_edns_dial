/*
 * tree.c
 *
 *  Created on: 2012-6-6
 *      Author: Administrator
 */

#include <clib/tree.h>
#include <clib/memcache.h>
#include <clib/memory.h>
#include <stdlib.h>
#include <clib/log.h>

#define MAX_TREE_HEIGHT 40

struct _tree_node {
    void *key; /* key for this node */
    void *value; /* value stored at this node */
    struct _tree_node *left; /* left subtree */
    struct _tree_node *right; /* right subtree */
    int8_t balance; /* height (right) - height (left) */
    uint8_t left_child;
    uint8_t right_child;
};

struct _tree_t {
    struct _tree_node *root;
    compare_func key_compare;
    void *user_data;
    destroy_func key_destroy_func;
    destroy_func value_destroy_func;
    uint nnodes;
    mem_cache_t* mem_cache;
};

tree_t* tree_new_full(uint size, compare_func key_compare_func, void *user_data,
        destroy_func key_destroy_func, destroy_func value_destroy_func)
{
    tree_t *tree;

    if (key_compare_func == NULL) {
        LOG(LOG_LEVEL_ERROR, "比较函数不能为空");
        return NULL;
    }

    tree = (tree_t *) mem_alloc(sizeof(tree_t));
    tree->root = NULL;
    tree->key_compare = key_compare_func;
    tree->user_data = user_data;
    tree->key_destroy_func = key_destroy_func;
    tree->value_destroy_func = value_destroy_func;
    tree->nnodes = 0;
    tree->mem_cache = mem_cache_create(sizeof(struct _tree_node), size);

    return tree;
}

tree_t* tree_new(uint size, compare_func key_compare_func, void *user_data)
{
    return tree_new_full(size, key_compare_func, user_data, NULL, NULL);
}

static inline struct _tree_node* tree_first_node(tree_t *tree)
{
    struct _tree_node *tmp;

    if (!tree->root)
        return NULL;

    tmp = tree->root;

    while (tmp->left_child)
        tmp = tmp->left;

    return tmp;
}

static inline struct _tree_node* tree_last_node(tree_t *tree)
{
    struct _tree_node *tmp;

    if (!tree->root)
        return NULL;

    tmp = tree->root;

    while (tmp->right_child)
        tmp = tmp->right;

    return tmp;
}

static inline struct _tree_node* tree_node_previous(struct _tree_node *node)
{
    struct _tree_node *tmp;

    tmp = node->left;

    if (node->left_child)
        while (tmp->right_child)
            tmp = tmp->right;

    return tmp;
}

static inline struct _tree_node* tree_node_next(struct _tree_node *node)
{
    struct _tree_node *tmp;

    tmp = node->right;

    if (node->right_child)
        while (tmp->left_child)
            tmp = tmp->left;

    return tmp;
}

void tree_remove_all(tree_t *tree)
{
    struct _tree_node *node;
    struct _tree_node *next;

    RETURN_IF_FAIL(tree != NULL);

    node = tree_first_node(tree);

    while (node) {
        next = tree_node_next(node);

        if (tree->key_destroy_func)
            tree->key_destroy_func(node->key);
        if (tree->value_destroy_func)
            tree->value_destroy_func(node->value);
        mem_cache_free(tree->mem_cache, node);

        node = next;
    }

    tree->root = NULL;
    tree->nnodes = 0;
}
void tree_free(tree_t *tree)
{
    RETURN_IF_FAIL(tree != NULL);

    tree_remove_all(tree);
    mem_cache_destroy(tree->mem_cache);
    mem_free(tree);
}

static struct _tree_node* tree_node_new(tree_t *tree, void *key, void * value)
{
    struct _tree_node *node = mem_cache_alloc(tree->mem_cache);

    node->balance = 0;
    node->left = NULL;
    node->right = NULL;
    node->left_child = false;
    node->right_child = false;
    node->key = key;
    node->value = value;

    return node;
}

uint tree_height(tree_t *tree)
{
    struct _tree_node *node;
    uint height;

    RETURN_VAL_IF_FAIL(tree != NULL, 0);

    if (!tree->root)
        return 0;

    height = 0;
    node = tree->root;

    while (1) {
        height += 1 + MAX(node->balance, 0);

        if (!node->left_child)
            return height;

        node = node->left;
    }
}

uint tree_nnodes(tree_t *tree)
{
    RETURN_VAL_IF_FAIL(tree != NULL, 0);

    return tree->nnodes;
}
static struct _tree_node* tree_node_rotate_left(struct _tree_node *node)
{
    struct _tree_node *right;
    int a_bal;
    int b_bal;

    right = node->right;

    if (right->left_child)
        node->right = right->left;
    else {
        node->right_child = false;
        right->left_child = true;
    }
    right->left = node;

    a_bal = node->balance;
    b_bal = right->balance;

    if (b_bal <= 0) {
        if (a_bal >= 1)
            right->balance = b_bal - 1;
        else
            right->balance = a_bal + b_bal - 2;
        node->balance = a_bal - 1;
    } else {
        if (a_bal <= b_bal)
            right->balance = a_bal - 2;
        else
            right->balance = b_bal - 1;
        node->balance = a_bal - b_bal - 1;
    }

    return right;
}

static struct _tree_node* tree_node_rotate_right(struct _tree_node *node)
{
    struct _tree_node *left;
    int a_bal;
    int b_bal;

    left = node->left;

    if (left->right_child)
        node->left = left->right;
    else {
        node->left_child = false;
        left->right_child = true;
    }
    left->right = node;

    a_bal = node->balance;
    b_bal = left->balance;

    if (b_bal <= 0) {
        if (b_bal > a_bal)
            left->balance = b_bal + 1;
        else
            left->balance = a_bal + 2;
        node->balance = a_bal - b_bal + 1;
    } else {
        if (a_bal <= -1)
            left->balance = b_bal + 1;
        else
            left->balance = a_bal + b_bal + 2;
        node->balance = a_bal + 1;
    }

    return left;
}

#ifdef G_TREE_DEBUG
static int
tree_node_height (struct _tree_node *node)
{
    int left_height;
    int right_height;

    if (node)
    {
        left_height = 0;
        right_height = 0;

        if (node->left_child)
        left_height = tree_node_height (node->left);

        if (node->right_child)
        right_height = tree_node_height (node->right);

        return MAX (left_height, right_height) + 1;
    }

    return 0;
}

static void tree_node_check (struct _tree_node *node)
{
    int left_height;
    int right_height;
    int balance;
    struct _tree_node *tmp;

    if (node)
    {
        if (node->left_child)
        {
            tmp = tree_node_previous (node);
            assert (tmp->right == node);
        }

        if (node->right_child)
        {
            tmp = tree_node_next (node);
            assert (tmp->left == node);
        }

        left_height = 0;
        right_height = 0;

        if (node->left_child)
        left_height = tree_node_height (node->left);
        if (node->right_child)
        right_height = tree_node_height (node->right);

        balance = right_height - left_height;
        assert (balance == node->balance);

        if (node->left_child)
        tree_node_check (node->left);
        if (node->right_child)
        tree_node_check (node->right);
    }
}

static void
tree_node_dump (struct _tree_node *node,
        int indent)
{
    print ("%*s%c\n", indent, "", *(char *)node->key);

    if (node->left_child)
    tree_node_dump (node->left, indent + 2);
    else if (node->left)
    print ("%*s<%c\n", indent + 2, "", *(char *)node->left->key);

    if (node->right_child)
    tree_node_dump (node->right, indent + 2);
    else if (node->right)
    print ("%*s>%c\n", indent + 2, "", *(char *)node->right->key);
}

void
tree_dump (GTree *tree)
{
    if (tree->root)
    tree_node_dump (tree->root, 0);
}
#endif

static struct _tree_node* tree_node_balance(struct _tree_node *node)
{
    if (node->balance < -1) {
        if (node->left->balance > 0)
            node->left = tree_node_rotate_left(node->left);
        node = tree_node_rotate_right(node);
    } else if (node->balance > 1) {
        if (node->right->balance < 0)
            node->right = tree_node_rotate_right(node->right);
        node = tree_node_rotate_left(node);
    }

    return node;
}

static void tree_insert_internal(tree_t *tree, void *key, void *value,
        bool replace)
{
    struct _tree_node *node;
    struct _tree_node *path[MAX_TREE_HEIGHT];
    int idx;

    RETURN_IF_FAIL(tree != NULL);

    if (!tree->root) {
        tree->root = tree_node_new(tree, key, value);
        tree->nnodes++;
        return;
    }

    idx = 0;
    path[idx++] = NULL;
    node = tree->root;

    while (1) {
        int cmp = tree->key_compare(key, node->key, tree->user_data);

        if (cmp == 0) {
            if (tree->value_destroy_func)
                tree->value_destroy_func(node->value);

            node->value = value;

            if (replace) {
                if (tree->key_destroy_func)
                    tree->key_destroy_func(node->key);

                node->key = key;
            } else {
                /* free the passed key */
                if (tree->key_destroy_func)
                    tree->key_destroy_func(key);
            }

            return;
        } else if (cmp < 0) {
            if (node->left_child) {
                path[idx++] = node;
                node = node->left;
            } else {
                struct _tree_node *child = tree_node_new(tree, key, value);

                child->left = node->left;
                child->right = node;
                node->left = child;
                node->left_child = true;
                node->balance -= 1;

                tree->nnodes++;

                break;
            }
        } else {
            if (node->right_child) {
                path[idx++] = node;
                node = node->right;
            } else {
                struct _tree_node *child = tree_node_new(tree, key, value);

                child->right = node->right;
                child->left = node;
                node->right = child;
                node->right_child = true;
                node->balance += 1;

                tree->nnodes++;

                break;
            }
        }
    }

    /* restore balance. This is the goodness of a non-recursive
     implementation, when we are done with balancing we 'break'
     the loop and we are done. */
    while (1) {
        struct _tree_node *bparent = path[--idx];
        bool left_node = (bparent && node == bparent->left);

        if (node->balance < -1 || node->balance > 1) {
            node = tree_node_balance(node);
            if (bparent == NULL)
                tree->root = node;
            else if (left_node)
                bparent->left = node;
            else
                bparent->right = node;
        }

        if (node->balance == 0 || bparent == NULL)
            break;

        if (left_node)
            bparent->balance -= 1;
        else
            bparent->balance += 1;

        node = bparent;
    }
}

void tree_insert(tree_t *tree, void *key, void *value)
{
    RETURN_IF_FAIL(tree != NULL);

    tree_insert_internal(tree, key, value, false);
}

void tree_replace(tree_t *tree, void *key, void *value)
{
    RETURN_IF_FAIL(tree != NULL);

    tree_insert_internal(tree, key, value, true);
}

static bool tree_remove_node_internal(tree_t *tree, struct _tree_node *node,
        struct _tree_node *path[], int idx, bool steal, void **orikey,
        void **value)
{
    struct _tree_node *parent, *balance;
    bool left_node;

    RETURN_VAL_IF_FAIL(tree != NULL, false);

    if (!tree->root)
        return false;

    /* the following code is almost equal to tree_remove_node,
     except that we do not have to call tree_node_parent. */
    balance = parent = path[--idx];
    left_node = (parent && node == parent->left);

    if (!node->left_child) {
        if (!node->right_child) {
            if (!parent)
                tree->root = NULL;
            else if (left_node) {
                parent->left_child = false;
                parent->left = node->left;
                parent->balance += 1;
            } else {
                parent->right_child = false;
                parent->right = node->right;
                parent->balance -= 1;
            }
        } else /* node has a right child */
        {
            struct _tree_node *tmp = tree_node_next(node);
            tmp->left = node->left;

            if (!parent)
                tree->root = node->right;
            else if (left_node) {
                parent->left = node->right;
                parent->balance += 1;
            } else {
                parent->right = node->right;
                parent->balance -= 1;
            }
        }
    } else /* node has a left child */
    {
        if (!node->right_child) {
            struct _tree_node *tmp = tree_node_previous(node);
            tmp->right = node->right;

            if (parent == NULL)
                tree->root = node->left;
            else if (left_node) {
                parent->left = node->left;
                parent->balance += 1;
            } else {
                parent->right = node->left;
                parent->balance -= 1;
            }
        } else /* node has a both children (pant, pant!) */
        {
            struct _tree_node *prev = node->left;
            struct _tree_node *next = node->right;
            struct _tree_node *nextp = node;
            int old_idx = idx + 1;
            idx++;

            /* path[idx] == parent */
            /* find the immediately next node (and its parent) */
            while (next->left_child) {
                path[++idx] = nextp = next;
                next = next->left;
            }

            path[old_idx] = next;
            balance = path[idx];

            /* remove 'next' from the tree */
            if (nextp != node) {
                if (next->right_child)
                    nextp->left = next->right;
                else
                    nextp->left_child = false;
                nextp->balance += 1;

                next->right_child = true;
                next->right = node->right;
            } else
                node->balance -= 1;

            /* set the prev to point to the right place */
            while (prev->right_child)
                prev = prev->right;
            prev->right = next;

            /* prepare 'next' to replace 'node' */
            next->left_child = true;
            next->left = node->left;
            next->balance = node->balance;

            if (!parent)
                tree->root = next;
            else if (left_node)
                parent->left = next;
            else
                parent->right = next;
        }
    }

    /* restore balance */
    if (balance)
        while (1) {
            struct _tree_node *bparent = path[--idx];
            left_node = (bparent && balance == bparent->left);

            if (balance->balance < -1 || balance->balance > 1) {
                balance = tree_node_balance(balance);
                if (!bparent)
                    tree->root = balance;
                else if (left_node)
                    bparent->left = balance;
                else
                    bparent->right = balance;
            }

            if (balance->balance != 0 || !bparent)
                break;

            if (left_node)
                bparent->balance += 1;
            else
                bparent->balance -= 1;

            balance = bparent;
        }

    if (!steal) {
        if (tree->key_destroy_func)
            tree->key_destroy_func(node->key);
        if (tree->value_destroy_func)
            tree->value_destroy_func(node->value);
    } else {
        if (orikey != NULL) {
            *orikey = node->key;
        }
        if (value != NULL) {
            *value = node->value;
        }
    }

    mem_cache_free(tree->mem_cache, node);

    tree->nnodes--;

    return true;
}

static bool tree_remove_internal(tree_t *tree, const void *key, bool steal,
        void **orikey, void **value)
{
    struct _tree_node *node;
    struct _tree_node *path[MAX_TREE_HEIGHT];
    int idx;

    RETURN_VAL_IF_FAIL(tree != NULL, false);

    if (!tree->root)
        return false;

    idx = 0;
    path[idx++] = NULL;
    node = tree->root;

    while (1) {
        int cmp = tree->key_compare(key, node->key, tree->user_data);

        if (cmp == 0)
            break;
        else if (cmp < 0) {
            if (!node->left_child)
                return false;

            path[idx++] = node;
            node = node->left;
        } else {
            if (!node->right_child)
                return false;

            path[idx++] = node;
            node = node->right;
        }
    }

    return tree_remove_node_internal(tree, node, path, idx, steal, orikey,
            value);
}

bool tree_remove(tree_t *tree, const void *key)
{
    bool removed;

    RETURN_VAL_IF_FAIL(tree != NULL, false);

    removed = tree_remove_internal(tree, key, false, NULL, NULL);

    return removed;
}

bool tree_steal(tree_t *tree, const void *lookup_key, void **orikey,
        void **value)
{
    bool stealed;

    RETURN_VAL_IF_FAIL(tree != NULL, false);

    stealed = tree_remove_internal(tree, lookup_key, true, orikey, value);

    return stealed;
}

static struct _tree_node * tree_find_node(tree_t *tree, const void *key)
{
    struct _tree_node *node;
    int cmp;

    node = tree->root;
    if (!node)
        return NULL;

    while (1) {
        cmp = tree->key_compare(key, node->key, tree->user_data);
        if (cmp == 0)
            return node;
        else if (cmp < 0) {
            if (!node->left_child)
                return NULL;

            node = node->left;
        } else {
            if (!node->right_child)
                return NULL;

            node = node->right;
        }
    }
    return NULL;
}

bool tree_lookup_extended(tree_t *tree, const void *lookup_key, void **orig_key,
        void **value)
{
    struct _tree_node *node;

    RETURN_VAL_IF_FAIL(tree != NULL, false);

    node = tree_find_node(tree, lookup_key);

    if (node) {
        if (orig_key)
            *orig_key = node->key;
        if (value)
            *value = node->value;
        return true;
    } else
        return false;
}

void *tree_lookup(tree_t *tree, const void *key)
{
    struct _tree_node *node;

    RETURN_VAL_IF_FAIL(tree != NULL, NULL);

    node = tree_find_node(tree, key);
    return node ? node->value : NULL;
}

void tree_foreach(tree_t *tree, traverse_pair_func func, void *user_data)
{
    struct _tree_node *node;

    RETURN_IF_FAIL(tree != NULL);

    if (!tree->root)
        return;

    node = tree_first_node(tree);

    while (node) {
        if ((*func)(node->key, node->value, user_data))
            break;

        node = tree_node_next(node);
    }
}

void tree_reverse_foreach(tree_t *tree, traverse_pair_func func,
        void *user_data)
{
    struct _tree_node *node;

    RETURN_IF_FAIL(tree != NULL);

    if (!tree->root)
        return;

    node = tree_last_node(tree);

    while (node) {
        if ((*func)(node->key, node->value, user_data))
            break;

        node = tree_node_previous(node);
    }
}

void tree_destroy_func(void* data)
{
    tree_free((tree_t*) data);
}

//下面两个函数有待优化，否则topn用的时候，如果有频繁的删除操作，会有效率问题
void * tree_remove_first_node(tree_t *tree)
{
    RETURN_VAL_IF_FAIL(tree != NULL, NULL);

    struct _tree_node *first_node;
    int idx;
    struct _tree_node *path[MAX_TREE_HEIGHT];
    struct _tree_node* temp_node;

    if (!tree->root)
        return NULL;

    idx = 0;
    path[idx++] = NULL;
    first_node = tree->root;

    while (first_node->left_child) {
        path[idx++] = first_node;
        first_node = first_node->left;
    }

    temp_node = tree_node_next(first_node);

    tree_remove_node_internal(tree, first_node, path, idx, false, NULL, NULL);

    return temp_node->key;
}

void * tree_remove_last_node(tree_t *tree)
{
    RETURN_VAL_IF_FAIL(tree != NULL, NULL);

    struct _tree_node *last_node;
    int idx;
    struct _tree_node *path[MAX_TREE_HEIGHT];
    struct _tree_node* temp_node;

    if (!tree->root)
        return NULL;

    idx = 0;
    path[idx++] = NULL;
    last_node = tree->root;

    while (last_node->right_child) {
        path[idx++] = last_node;
        last_node = last_node->right;
    }

    temp_node = tree_node_previous(last_node);

    tree_remove_node_internal(tree, last_node, path, idx, false, NULL, NULL);

    return temp_node->key;
}
