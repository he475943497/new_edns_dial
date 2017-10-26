#include <clib/topn.h>
#include <clib/string.h>
#include <clib/log.h>
#include <stdio.h>
#include <string.h>
#include <clib/memory.h>

static bool my_traverse(void *key, void *value, void *user_data)
{
    printf("%d\t%s\n", *(int*) key, (char*) value);
    return false;
}

void test_topn()
{
    int *key;

    topn_t* topn = topn_new(5, int_compare_func, NULL, default_destroy_func,
            default_destroy_func);

    key = (int*) mem_alloc(sizeof(int));
    *key = 4;
    topn_insert(topn, key, strdup2("hello4"));

    key = (int*) mem_alloc(sizeof(int));
    *key = 87;
    topn_insert(topn, key, strdup2("hello87"));

    key = (int*) mem_alloc(sizeof(int));
    *key = 4;
    topn_insert(topn, key, strdup2("hello4-2"));

    key = (int*) mem_alloc(sizeof(int));
    *key = 2;
    topn_insert(topn, key, strdup2("hello2"));

    key = (int*) mem_alloc(sizeof(int));
    *key = 1;
    topn_insert(topn, key, strdup2("hello1"));

    key = (int*) mem_alloc(sizeof(int));
    *key = 565;
    topn_insert(topn, key, strdup2("hello565"));

    key = (int*) mem_alloc(sizeof(int));
    *key = 564;
    topn_insert(topn, key, strdup2("hello564"));

    LOG(LOG_LEVEL_TEST, "topn size: %d", topn_size(topn));

    topn_foreach(topn, my_traverse, NULL);

    topn_free(topn);
}
