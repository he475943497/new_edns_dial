/*
 * test_string.c
 *
 *  Created on: 2012-6-1
 *      Author: Administrator
 */

#include <clib/string.h>
#include <clib/types.h>
#include <clib/log.h>
#include <string.h>
#include <clib/memory.h>
#include <stdio.h>

static void test_strtrim()
{
    char buf[256] = "abc";
    char buf1[256] = "abc";
    char buf2[256] = " abc";
    char buf3[256] = "abc ";
    char buf4[256] = "   ";
    char buf5[256] = "";

    if (strcmp(strtrim(buf1, strlen(buf1)), buf) != 0) {
        LOG(LOG_LEVEL_TEST, "test_strtrim:%s", buf1);
    }

    if (strcmp(strtrim(buf2, strlen(buf2)), buf) != 0) {
        LOG(LOG_LEVEL_TEST, "test_strtrim:%s", buf2);
    }
    if (strcmp(strtrim(buf3, strlen(buf3)), buf) != 0) {
        LOG(LOG_LEVEL_TEST, "test_strtrim:%s", buf3);
    }
    if (strcmp(strtrim(buf4, strlen(buf4)), "") != 0) {
        LOG(LOG_LEVEL_TEST, "test_strtrim:%s", buf4);
    }
    if (strcmp(strtrim(buf5, strlen(buf5)), "") != 0) {
        LOG(LOG_LEVEL_TEST, "test_strtrim:%s", buf5);
    }
}
static void test_strsplit()
{
    char buf[256] = " a  b  c ";
//    char buf1[256] = "  a  b   c  ";
//    char buf2[256] = " a b  c";
//    char buf3[256] = "a b c ";
    char sep[] = " ";
    str_array_t *array;

//    list_t *result = list_new(8, default_destroy_func);
//    list_append(result, strdup("a"));
//    list_append(result, strdup("b"));
//    list_append(result, strdup("c"));

    array = strsplit(buf, sep);
    for (int i = 0; i < str_array_size(array); i++) {
        printf("--%s--\t", str_array_at(array, i));
    }

    str_array_free(array);
}
static void test_str_tolower()
{
    char buf[]="HelLO";
    str_tolower(buf);
    printf("%s\n", buf);
}

/*
 * add some api -- rongyi
 */
static void test_new()
{
    char *str = "helloworld";
    char *str_eq = "helloworld";
    char *str_another = "helloword";
    char *prefix = "hell";
    char *prefix2 = "hhell";
    char *postfix = "ld";
    char *postfix2 = "lld";

    if (streq(str, str_eq) && !streq(str, str_another))
        printf("test streq success\n");

    if (strstarts(str, prefix) && !strstarts(str, prefix2))
        printf("test strstarts success\n");

    if (strends(str, postfix) && !strstarts(str, postfix2))
        printf("test strends success\n");

    printf("test stringify %s\n", stringify(str_eq));

    char *haystack = "abcababc ab c";
    char *needle1 = "abc";
    char *needle2 = "ab";
    if (strcount(haystack, needle1) == 2
            && strcount(haystack, needle2) == 4)
        printf("test strcount success\n");
}

void test_string()
{
    /* test_str_tolower(); */
    test_new();
//    test_strtrim();
//    test_strsplit();
}
