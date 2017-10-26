/*
 * test_seg_tree.c
 *
 *  Created on: 2012-6-8
 *      Author: Administrator
 */
#include <clib/seg_tree.h>
#include <clib/list.h>
#include <clib/string.h>
#include <clib/log.h>
#include <stdio.h>
#include <string.h>

static bool my_traverse_func(uint start, uint end, list_t* value, void* data)
{
    printf("%d - %d:\t", start, end);

    struct list_node *node = list_head(value);
    while (node != NULL) {
        printf("%s\t", (char*) (node->data));
        node = node->next;
    }

    printf("\n");

    return false;
}

static bool my_traverse_func1(void *data, void* user_data)
{
    printf("%s\t", (char*) data);
    return false;
}

void test_seg_tree1()
{
    seg_tree_t *tree = seg_tree_new(default_destroy_func);

    seg_tree_insert(tree, 0, 2147483647u, strdup2("hello1"));
    seg_tree_insert(tree, 2030764032u, 2030829567u, strdup2("hello2"));
    seg_tree_insert(tree, 3702849536u, 3702915071u, strdup2("hello3"));
    seg_tree_insert(tree, 977207296u, 977272831u, strdup2("hello4"));
    seg_tree_insert(tree, 3029663744u, 3029729279, strdup2("hello5"));
//    seg_tree_insert(tree, 12345, 23456, strdup("hello5"));
//    seg_tree_insert(tree, 6, 8, strdup("hello12"));
//    seg_tree_insert(tree, 5, 7, strdup("hello11"));

    seg_tree_foreach(tree, my_traverse_func, NULL);

    list_t *new_list = seg_tree_lookup(tree, 2030764034);
    if (NULL != new_list) {
        printf("-----------------------------------\n");
        list_foreach(new_list, my_traverse_func1, NULL);
        printf("-----------------------------------\n");
        list_free(new_list);
    }
    seg_tree_free(tree);
}

void test_seg_tree()
{
    test_seg_tree1();
    test_seg_tree1();
    test_seg_tree1();
    test_seg_tree1();
}
