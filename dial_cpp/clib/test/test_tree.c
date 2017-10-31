/*
 * test_tree.c
 *
 *  Created on: 2012-6-8
 *      Author: Administrator
 */

#include <clib/tree.h>
#include <clib/string.h>
#include <clib/log.h>
#include <stdio.h>
#include <string.h>

static void test_str_str_tree()
{
    tree_t *tree = tree_new_full(10, str_compare_func, NULL,
            default_destroy_func, default_destroy_func);
    char *find;

    tree_insert(tree, strdup2("key1"), strdup2("value1"));
    tree_insert(tree, strdup2("key2"), strdup2("value2"));
    tree_insert(tree, strdup2("key3"), strdup2("value3"));
    tree_insert(tree, strdup2("key4"), strdup2("value4"));
    tree_insert(tree, strdup2("key2"), strdup2("value22"));

    if (tree_nnodes(tree) != 4) {
        LOG(LOG_LEVEL_TEST, "tree_size 错误");
    }

    find = tree_lookup(tree, "key2");
    if (strcmp(find, "value22") != 0) {
        LOG(LOG_LEVEL_TEST, "tree_lookup 错误");
    }

    find = tree_lookup(tree, "key3");
    if (strcmp(find, "value3") != 0) {
        LOG(LOG_LEVEL_TEST, "tree_lookup 错误");
    }

    tree_free(tree);
}

void test_tree()
{
    test_str_str_tree();
}
