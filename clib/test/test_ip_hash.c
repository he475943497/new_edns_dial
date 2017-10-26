#include "clib/ip_hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <clib/log.h>
#include <string.h>
#include <clib/memory.h>
#include <clib/string.h>
#include <clib/list.h>


bool printv(void *data, void *user_data)
{
    int *p;
    p = data;
    printf("value:%d\n", *p);
    return false;
}

void test_ip_hash()
{
    int *pi;
    int i = 77677;
    int b = 73637;

    ip_hash_t *p;
    p = ip_hash_init();

    ip_hash_add(p, 1000, 2000, (void *)&i);
    ip_hash_add(p, 1000, 2000, (void *)&b);
    
    list_t *nd = ip_hash_search(p, 1500);
    printf("size:%d\n", list_size(nd));
    list_foreach(nd, printv, NULL);
    ip_hash_uninit(p);

    return ;
}

