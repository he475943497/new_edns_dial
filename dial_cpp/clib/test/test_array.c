#include "clib/array.h"

#include <stdio.h>
#include <stdlib.h>
#include <clib/log.h>
#include <string.h>
#include <clib/memory.h>
#include <clib/string.h>

int da[5] = { 1, 2, 3, 4, 5 };
int ba[5] = { 8, 8, 8, 8, 8 };
int ca[5] = { 7, 7, 7, 7, 7 };

void test_org_array()
{
    array_t *ary;
    ary = array_new(sizeof(int));
    if (ary == NULL)
        printf("errror\n");
    array_append_vals(ary, da, 5);
    array_prepend_vals(ary, ba, 5);
    //array_dump(ary);
    printf("======================\n");
    array_insert_vals(ary, 3, ca, 5);
    //array_dump(ary);
    printf("======================\n");
    array_remove_range(ary, 8, 2);
    //array_dump(ary);
}

void test_str_array()
{
    str_array_t *ary;
    str_array_t *arydup;
    ary = str_array_new_full(16);
    str_array_append(ary, strdup2("1111"));
    str_array_prepend(ary, strdup2("2222"));
    str_array_insert(ary, 1, strdup2("3333"));
    for (int i = 0; i < str_array_size(ary); i++) {
        LOG(LOG_LEVEL_TEST, str_array_at(ary, i));
    }
    LOG(LOG_LEVEL_TEST, "after dup -----------------------");
    //    str_array_remove_range(ary, 0, 2);
    arydup = str_array_dup(ary, NULL);
    for (int i = 0; i < str_array_size(arydup); i++) {
        LOG(LOG_LEVEL_TEST, str_array_at(arydup, i));
    }
    str_array_free(arydup);
    str_array_free(ary);
}

void test_int_array()
{
    int_array_t *ary;

    ary = int_array_new();
    int_array_append(ary, 1);
    int_array_append(ary, 2);
    int_array_append(ary, 3);
    int_array_append(ary, 4);
    int_array_append(ary, 5);
    int_array_append(ary, 5);
    int_array_append(ary, 6);
    int_array_append(ary, 7);
    int_array_append(ary, 8);
    int_array_append(ary, 9);

    int_array_insert(ary, 1, 0);

    int_array_insert(ary, 15, 0);
    int_array_insert(ary, 2, 0);
    for (int i = 0; i < int_array_size(ary); i++) {
        LOG(LOG_LEVEL_TEST, "%d", int_array_at(ary, i));
    }
    LOG(LOG_LEVEL_TEST, "-----------------------");
    int_array_remove_range(ary, 0, 11);
    //int_array_remove_index(ary, 11);

    for (int i = 0; i < int_array_size(ary); i++) {
        LOG(LOG_LEVEL_TEST, "%d", int_array_at(ary, i));
    }
    LOG(LOG_LEVEL_TEST, "-----------------------");
    int_array_free(ary);
}

struct ip_seg {
    int start;
    int end;
};
void test_ptr_array()
{
    ptr_array_t *ary;
    struct ip_seg *temp_seg;

    ary = ptr_array_new_full(8, default_destroy_func);

    temp_seg = (struct ip_seg *) mem_alloc(sizeof(struct ip_seg ));
    temp_seg->start = 3;
    temp_seg->end = 5;
    ptr_array_append(ary, temp_seg);

    temp_seg = (struct ip_seg *) mem_alloc(sizeof(struct ip_seg ));
    temp_seg->start = 5;
    temp_seg->end = 7;
    ptr_array_append(ary, temp_seg);

    temp_seg = (struct ip_seg *) mem_alloc(sizeof(struct ip_seg ));
    temp_seg->start = 134;
    temp_seg->end = 213;
    ptr_array_append(ary, temp_seg);

    for (int i = 0; i < ptr_array_size(ary); i++) {
        temp_seg = ptr_array_at(ary, i);
        LOG(LOG_LEVEL_TEST, "%d - %d", temp_seg->start, temp_seg->end);
    }
    LOG(LOG_LEVEL_TEST, "-----------------------");

//    for (int i = 0; i < int_array_size(ary); i++) {
//        LOG(LOG_LEVEL_TEST, "%d", int_array_at(ary, i));
//    }
//    LOG(LOG_LEVEL_TEST, "-----------------------");
    ptr_array_free(ary);
}

void test_array()
{
//    test_int_array();
//    test_str_array();
    test_ptr_array();
}

