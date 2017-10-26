/*
 * test_hash_table.c
 *
 *  Created on: 2012-6-8
 *      Author: Administrator
 */
#include <clib/process_info.h>
#include <clib/hash_table.h>
#include <clib/string.h>
#include <clib/log.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

//static bool my_traverse(void *key, void *value, void *user_data)
//{
//    printf("%s\t%s\n", (char*)key, (char*)value);
//    return false;
//}

static void test_str_str_hash_table()
{
    hash_table_t *hash_table = hash_table_new(str_hash, str_compare_func, NULL,
            default_destroy_func, default_destroy_func);
    char *find;

    hash_table_insert(hash_table, strdup2("key1"), strdup2("value1"));
    hash_table_insert(hash_table, strdup2("key2"), strdup2("value2"));
    hash_table_insert(hash_table, strdup2("key3"), strdup2("value3"));
    hash_table_insert(hash_table, strdup2("key4"), strdup2("value4"));
    hash_table_insert(hash_table, strdup2("key2"), strdup2("value22"));
    int ret = hash_table_contains(hash_table, "key1");

    if (hash_table_size(hash_table) != 4) {
        LOG(LOG_LEVEL_TEST, "hash_table_size 错误");
    }

    //hash_table_foreach(hash_table, my_traverse, NULL);
    find = hash_table_lookup(hash_table, "key2");
    if (find == NULL || strcmp(find, "value22") != 0) {
        LOG(LOG_LEVEL_TEST, "hash_table_lookup 错误");
    }

    find = hash_table_lookup(hash_table, "key3");
    if (find == NULL || strcmp(find, "value3") != 0) {
        LOG(LOG_LEVEL_TEST, "hash_table_lookup 错误");
    }

    hash_table_free(hash_table);
}

void test_hash_table_N()
{
    int i;
    char dname[255];
    struct proc_stat stat;
    void *key;
    void *value;
    hash_table_t *hash_table = hash_table_new(str_hash, str_compare_func, NULL,
            default_destroy_func, NULL);
    struct timespec time1 = { 0, 0 };
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    printf("CLOCK_PROCESS_CPUTIME_ID: %ld, %ld\n", time1.tv_sec, time1.tv_nsec);
    for (i = 0; i < 1000000; i++) {
        //bzero(dname, sizeof(dname));
        snprintf(dname, 255, "www.renren.com_%d\n", i);
        hash_table_insert(hash_table, strdup2(dname), NULL);
        //printf("dname = %s",dname);
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    printf("CLOCK_PROCESS_CPUTIME_ID: %ld, %ld\n", time1.tv_sec, time1.tv_nsec);
    for (i = 0; i < 1000000; i++) {
        //bzero(dname, sizeof(dname));
        //snprintf(dname, 255, "www.renren.com_%d\n", i);
        if (!hash_table_lookup_extended(hash_table, "www.renren.com_100\n",
                &key, &value)) {
            printf("dname =========== %s", dname);
            break;
        }
        //printf("dname = %s",dname);
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    printf("CLOCK_PROCESS_CPUTIME_ID: %ld, %ld\n", time1.tv_sec, time1.tv_nsec);
    if (0 == process_stat(getpid(), &stat))
        printf("===========mem_size = %u==============\n", stat.mem_size);
    hash_table_free(hash_table);

}

void test_hash_table_8()
{
    hash_table_t *hash_table = hash_table_new(str_hash, str_compare_func, NULL,
            default_destroy_func, NULL);
    hash_table_insert(hash_table, strdup2("1"), NULL);
    hash_table_insert(hash_table, strdup2("2"), NULL);
    hash_table_insert(hash_table, strdup2("3"), NULL);
    hash_table_insert(hash_table, strdup2("4"), NULL);
    hash_table_insert(hash_table, strdup2("5"), NULL);
    hash_table_insert(hash_table, strdup2("6"), NULL);
    hash_table_insert(hash_table, strdup2("7"), NULL);
    hash_table_insert(hash_table, strdup2("8"), NULL);

    hash_table_remove(hash_table, "1");
    hash_table_remove(hash_table, "2");
    hash_table_remove(hash_table, "3");
    hash_table_remove(hash_table, "4");
    hash_table_remove(hash_table, "5");
    hash_table_remove(hash_table, "6");
    hash_table_remove(hash_table, "7");
    hash_table_remove(hash_table, "8");

    hash_table_insert(hash_table, strdup2("1"), NULL);

    hash_table_free(hash_table);

}

void test_hash_table()
{
    test_str_str_hash_table();
    //test_hash_table_8();
}

