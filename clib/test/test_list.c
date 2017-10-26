
#include <clib/types.h>
#include <clib/string.h>
#include <clib/list.h>
#include <clib/log.h>


static bool print_list(void *data, void *user_data)
{
    LOG(LOG_LEVEL_TEST, (char*)data);
    return false;
}

void test_append()
{
    list_t* list = list_new(5, default_destroy_func);

    list_append(list, strdup2("hello1"));
    list_append(list, strdup2("hello2"));
    list_append(list, strdup2("hello3"));
    list_append(list, strdup2("hello4"));
    list_append(list, strdup2("hello5"));
    list_append(list, strdup2("hello6"));

    list_foreach(list, print_list, NULL);

    list_free(list);
}

void test_remove()
{
    list_t* list = list_new(1, default_destroy_func);

    list_append(list, strdup2("hello1"));
    list_foreach(list, print_list, NULL);
    list_remove_node(list, list_head(list));
    list_foreach(list, print_list, NULL);

    list_free(list);
}

void test_copy()
{
    list_t* list = list_new(5, default_destroy_func);
    list_t* new_list;
    list_append(list, strdup2("hello1"));
    list_append(list, strdup2("hello2"));
    list_append(list, strdup2("hello3"));
    list_append(list, strdup2("hello4"));
    list_append(list, strdup2("hello5"));
    list_append(list, strdup2("hello6"));

    new_list = list_duplicate(list, NULL);

    LOG(LOG_LEVEL_TEST, "print new list");
    list_foreach(new_list, print_list, NULL);
    LOG(LOG_LEVEL_TEST, "print list");
    list_foreach(list, print_list, NULL);

    list_free(list);
    list_free(new_list);
}


void test_list()
{
//    test_append();
//    test_copy();
    test_remove();
}

