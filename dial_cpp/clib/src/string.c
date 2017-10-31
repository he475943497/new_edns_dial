/*
 * string.c
 *
 *  Created on: 2012-6-1
 *      Author: Administrator
 */
#include <string.h>
#include <stdlib.h>
#include <clib/memory.h>
#include <clib/array.h>
#include <ctype.h>

size_t strcount(const char *haystack, const char *needle)
{
    size_t i = 0;
    size_t len = strlen(needle);
    while ((haystack = strstr(haystack, needle)) != NULL) {
        ++i;
        haystack += len;
    }

    return i;
}

char * strtrim(char *str, int strlen)
{
    char *start = str;
    char *end = &str[strlen - 1];

    if (str == NULL) {
        return NULL;
    }

    while (end != start) { //结束空格
        if (isspace(*end)) {
            *end = '\0';
        } else {
            break;
        }
        end--;
    }

    while (start != '\0') { //起始空格
        if (isspace(*start))
            start++;
        else
            break;
    }

    return start;
}

void* memdup(const void *mem, uint size)
{
    void *new_mem;

    if (mem) {
        new_mem = mem_alloc(size);
        memcpy(new_mem, mem, size);
    } else
        new_mem = NULL;

    return new_mem;
}

char* strdup2(const char *src)
{
    if (src == NULL) {
        return NULL;
    }
    int len = strlen(src);
    char *dst = (char*) mem_alloc(len + 1);
    memcpy(dst, src, len + 1);

    return dst;
}

//char * str_merge_space(const char *str)
//{
//    const char *src = str;
//    char *result, *dst;
//
//    if (str == NULL) {
//        return NULL;
//    }
//
//    result = (char*) mem_alloc(strlen(str)+1);
//    dst = result;
//    while (*src != '\0') {
//        if (isspace(*src)) {
//            if (dst != result && !isspace(dst - 1)) {
//                *(dst++) = *src;
//            }
//        } else {
//            *(dst++) = *src;
//        }
//        src++;
//    }
//
//    if (dst != result && isspace(dst - 1)) {
//        *(dst - 1) = '\0';
//    } else {
//        *(dst) = '\0';
//    }
//
//    return result;
//}

str_array_t* strsplit(const char *str, const char *sep)
{
    char *saveptr;
    char *src;
    char *token;

    if (str == NULL) {
        return NULL;
    }

    str_array_t* array = str_array_new_full(32);
    src = strdup2(str);

    token = (char*) strtok_r(src, sep, &saveptr);
    while (NULL != token) {
        str_array_append(array, strdup2(token));
        token = (char*) strtok_r(NULL, sep, &saveptr);
    }

    mem_free(src);

    return array;
}

void str_tolower(char *str)
{
    while (*str != '\0') {
        if (*str >= 'A' && *str <= 'Z') {
            *str += 'a' - 'A';
        }
        str++;
    }
}

char* strlast(char* str, char ch)
{
    if (str == NULL) {
        return NULL;
    }

    int len = strlen(str);
    for (int i = len - 1; i >= 0; i--) {
        if (str[i] == ch) {
            return &(str[i]);
        }
    }

    return NULL;
}

bool str_endwith(const char* str, const char* end)
{
    if (str == NULL || end == NULL) {
        return false;
    }

    int str_len = strlen(str);
    int end_len = strlen(end);
    if(str_len < end_len){
        return false;
    }

    if(strcmp(str+(str_len-end_len), end) == 0){
        return true;
    }

    return false;
}
