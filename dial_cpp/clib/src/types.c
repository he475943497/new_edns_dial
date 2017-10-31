#include <clib/types.h>
#include <stdlib.h>
#include <string.h>
#include <clib/memory.h>
#include <clib/socket.h>
#include <clib/log.h>

//#define BIGGER 1
//#define EQUAL   0
//#define SMALLER

void default_destroy_func(void* data)
{
    mem_free(data);
}

int int32_compare_func(const void *a, const void *b, void *user_data)
{
    int32_t v1 = *((const int32_t*) a);
    int32_t v2 = *((const int32_t*) b);

    if (v1 > v2) {
        return 1;
    } else if (v1 == v2) {
        return 0;
    } else {
        return -1;
    }
}
int uint32_compare_func(const void *a, const void *b, void *user_data)
{
    uint32_t v1 = *((const uint32_t*) a);
    uint32_t v2 = *((const uint32_t*) b);

    if (v1 > v2) {
        return 1;
    } else if (v1 == v2) {
        return 0;
    } else {
        return -1;
    }
}

int int_compare_func(const void *a, const void *b, void *user_data)
{
    int v1 = *((const int*) a);
    int v2 = *((const int*) b);

    if (v1 > v2) {
        return 1;
    } else if (v1 == v2) {
        return 0;
    } else {
        return -1;
    }
}

int uint_compare_func(const void *a, const void *b, void *user_data)
{
    uint v1 = *((const uint*) a);
    uint v2 = *((const uint*) b);

    if (v1 > v2) {
        return 1;
    } else if (v1 == v2) {
        return 0;
    } else {
        return -1;
    }
}

int str_compare_func(const void *a, const void *b, void *user_data)
{
    if (a == b) {
        return 0;
    }
    const char *v1 = (const char*) a;
    const char *v2 = (const char*) b;

    return strcmp(v1, v2);
}

int double_compare_func(const void *a, const void *b, void *user_data)
{
    double v1 = *((const double*) a);
    double v2 = *((const double*) b);

    if (v1 > v2) {
        return 1;
    } else if (v1 == v2) {
        return 0;
    } else {
        return -1;
    }
}

int int64_compare_func(const void *a, const void *b, void *user_data)
{
    uint64_t v1 = *((const uint64_t*) a);
    uint64_t v2 = *((const uint64_t*) b);

    if (v1 > v2) {
        return 1;
    } else if (v1 == v2) {
        return 0;
    } else {
        return -1;
    }
}

int direct_compare_func(const void *a, const void *b, void *user_data)
{
    if (a > b) {
        return 1;
    } else if (a == b) {
        return 0;
    } else {
        return -1;
    }
}

int ip_compare_func(const void *a, const void *b, void *user_data)
{
    struct ip_addr *v1 = (struct ip_addr *) a;
    struct ip_addr *v2 = (struct ip_addr *) b;

    if (v1->version > v2->version) {
        return 1;
    } else if (v1->version < v2->version) {
        return -1;
    } else {
        if (v1->version == IPV4) {
            if (v1->addr.ipv4 > v2->addr.ipv4) {
                return 1;
            } else if (v1->addr.ipv4 == v2->addr.ipv4) {
                return 0;
            } else {
                return -1;
            }
        } else { //IPV6
            return memcmp(&v1->addr.ipv6, &v2->addr.ipv6, sizeof(v2->addr.ipv6));
        }
        return -1;
    }
}
