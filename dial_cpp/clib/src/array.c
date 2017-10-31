/*
 * array.c
 *
 *  Created on: 2012-5-31
 *      Author: Administrator
 */
#include <string.h>
#include <clib/string.h>
#include <stdlib.h>

#include "clib/types.h"
#include "clib/array.h"
#include "clib/memory.h"

struct _array {
    void *data;
    uint len;
    uint alloc_size;
    uint elt_size;
    uint reserved_size;
    destroy_func clear_func;
};

struct _ptr_array {
    struct _array array;
};

struct _int_array {
    struct _array array;
};

struct _str_array {
    struct _ptr_array array;
};

array_t* array_new(uint element_size)
{
    struct _array *ary;
    if (element_size == 0)
        return NULL;
    ary = (struct _array *) mem_alloc(sizeof(struct _array));
    if (ary == NULL)
        return NULL;
    ary->elt_size = element_size;
    ary->data = NULL;
    ary->len = 0;
    ary->alloc_size = 0;
    ary->reserved_size = 1;
    ary->clear_func = NULL;
    return ary;
}

array_t* array_new_sized(uint element_size, uint size)
{
    struct _array *ary;
    if (element_size == 0 || size == 0)
        return NULL;
    ary = (struct _array *) mem_alloc(sizeof(struct _array));
    if (ary == NULL)
        return NULL;
    ary->elt_size = element_size;
    ary->data = NULL;
    ary->len = 0;
    ary->alloc_size = 0;
    ary->reserved_size = size;
    ary->clear_func = NULL;
    return ary;

}

array_t* array_new_full(uint element_size, uint size, destroy_func clear_func)
{
    struct _array *ary;
    if (element_size == 0 || size == 0)
        return NULL;
    ary = (struct _array *) mem_alloc(sizeof(struct _array));
    if (ary == NULL)
        return NULL;
    ary->elt_size = element_size;
    ary->data = NULL;
    ary->len = 0;
    ary->alloc_size = 0;
    ary->reserved_size = size;
    ary->clear_func = clear_func;
    return ary;

}

void array_free(array_t *array)
{
    if (array != NULL) {
        if (array->data != NULL) {
            if (array->clear_func != NULL) {
                for (int i = 0; i < array->len; i++) {
                    array->clear_func(array->data + i * array->elt_size);
                }
            }
            free(array->data);
            array->data = NULL;
        }
        mem_free(array);
    }
}

array_t* array_append_vals(array_t *array, const void *data, uint len)
{
    int i;
    void *ptr;
    if (array == NULL)
        return NULL;
    if (data == NULL || len == 0)
        return array;
    if (array->alloc_size - array->len < len) {
        i = (len - (array->alloc_size - array->len)) / array->reserved_size + 1;
        if (array->data == NULL) {
            /*data后面有可能调用realloc，所以不使用mem_alloc*/
            array->data = malloc(i * array->reserved_size * array->elt_size);
            if (array->data == NULL) {
                return NULL;
            }
        } else {
            ptr = realloc(array->data,
                    i * array->reserved_size * array->elt_size
                            + array->alloc_size * array->elt_size);
            if (ptr == NULL)
                return NULL;
            else
                array->data = ptr;
        }
        array->alloc_size += i * array->reserved_size;
    }
    memcpy(array->data + array->len * array->elt_size, data,
            len * array->elt_size);
    array->len += len;
    return array;
}

array_t* array_prepend_vals(array_t *array, const void *data, uint len)
{
    uint i;
    void *ptr;
    if (array == NULL)
        return NULL;
    if (data == NULL || len == 0)
        return array;
    //printf("first size:%d, %d\n", array->alloc_size, array->len);
    if (array->alloc_size - array->len < len) {
        i = (len - (array->alloc_size - array->len)) / array->reserved_size + 1;
        if (array->data == NULL) {
            //printf("malloc size:%d, %d\n", array->alloc_size, array->len);
            /*data后面有可能调用realloc，所以不使用mem_alloc*/
            array->data = malloc(i * array->reserved_size * array->elt_size);
            if (array->data == NULL)
                return NULL;
        } else {
            //printf("realloc size:%d, %d\n", array->alloc_size, array->len);
            ptr = realloc(array->data,
                    i * array->reserved_size * array->elt_size
                            + array->alloc_size * array->elt_size);
            if (ptr == NULL)
                return NULL;
            else
                array->data = ptr;
            //printf("realloc size:%d, %d, realloc size:%d\n", array->alloc_size, array->len, i*array->reserved_size * array->elt_size + array->alloc_size*array->elt_size);
        }
        array->alloc_size += i * array->reserved_size;
    }
    for (uint j = array->len; j > 0; j--) {
        memcpy(array->data + (j - 1 + len) * array->elt_size,
                array->data + (j - 1) * array->elt_size, array->elt_size);
    }
    memcpy(array->data, data, len * array->elt_size);
    array->len += len;
    return array;
}

array_t* array_insert_vals(array_t *array, uint index, const void *data,
        uint len)
{
    void *ptr;
    uint i;
    if (array == NULL)
        return NULL;
    if (data == NULL || len == 0 || index >= array->len)
        return array;
    if (array->alloc_size - array->len < len) {
        i = (len - (array->alloc_size - array->len)) / array->reserved_size + 1;
        if (array->data == NULL) {
            array->data = malloc(i * array->reserved_size * array->elt_size);
            if (array->data == NULL)
                return NULL;
        } else {
            ptr = realloc(array->data,
                    i * array->reserved_size * array->elt_size
                            + array->alloc_size * array->elt_size);
            if (ptr == NULL)
                return NULL;
            else
                array->data = ptr;
        }
        array->alloc_size += i * array->reserved_size;
    }
    for (uint j = array->len; j > index; j--) {
        memcpy(array->data + (j - 1 + len) * array->elt_size,
                array->data + (j - 1) * array->elt_size, array->elt_size);
    }

    memcpy(array->data + index * array->elt_size, data, len * array->elt_size);
    array->len += len;
    return array;
}

array_t* array_append_val(array_t *array, const void *data)
{
    return array_append_vals(array, data, 1);
}

array_t* array_prepend_val(array_t *array, const void *data)
{
    return array_prepend_vals(array, data, 1);
}

array_t* array_insert_val(array_t *array, uint index, const void *data)
{
    return array_insert_vals(array, index, data, 1);
}

array_t* array_set_size(array_t *array, uint length)
{
    if (array == NULL)
        return NULL;
    array->elt_size = length;
    return array;
}

array_t* array_remove_index(array_t *array, uint index)
{
    uint s;
    if (array == NULL)
        return NULL;
    if (array->data == NULL)
        return NULL;
    if (index >= array->len)
        return NULL;
    s = array->elt_size;
    if (array->clear_func != NULL)
        array->clear_func(array->data + s * index);
    for (int i = index; i < array->len - 1; i++)
        memcpy(array->data + i * s, array->data + (i + 1) * s, s);
    array->len -= 1;
    return array;
}

array_t* array_remove_range(array_t *array, uint index, uint length)
{
    uint s;
    if (array == NULL)
        return NULL;
    if (array->data == NULL)
        return NULL;
    if ((index + length) > array->len)
        return NULL;
    if (length == 0)
        return NULL;
    s = array->elt_size;
    if (array->clear_func != NULL) {
        for (int i = index; i < index + length; i++)
            array->clear_func(array->data + s * i);
    }
    for (int i = index; i < array->len; i++)
        memcpy(array->data + i * s, array->data + (i + length) * s, s);
    array->len -= length;
    return array;
}

uint array_size(array_t *array)
{
    if (array == NULL)
        return 0;
    return array->len;
}

void* array_at(array_t *array, uint index)
{
    uint s;
    if (array == NULL)
        return NULL;
    if (index >= array->len)
        return NULL;
    s = array->elt_size;
    return array->data + s * index;
}

void array_set(array_t *array, uint index, void* data)
{
    uint s;
    if (array == NULL)
        return;
    if (array->data == NULL)
        return;
    if (index > array->len)
        return;
    s = array->elt_size;
    memcpy(array->data + s * index, data, s);
}

array_t* array_duplicate(array_t* array, destroy_func clear_func)
{
    struct _array *ary;
    ary = (struct _array *) mem_alloc(sizeof(struct _array));
    memcpy(ary, (struct _array *) array, sizeof(struct _array));
    ary->clear_func = clear_func;
    if (ary->data != NULL) {
        ary->data = malloc(ary->alloc_size);
    }
    return ary;
}
/*
 int array_dump(array_t *ary)
 {
 for(int i = 0; i < ary->len; i++){
 printf("array index: %d, value: %d\n", i, *(int *)(ary->data+i*ary->elt_size));
 }
 }
 */

/*
 * **********************PTR ARRAY
 */

ptr_array_t* ptr_array_new()
{
    return (ptr_array_t*) array_new(sizeof(void*));
}

ptr_array_t* ptr_array_new_sized(uint size)
{
    return (ptr_array_t*) array_new_sized(sizeof(void*), size);
}

ptr_array_t* ptr_array_new_full(uint size, destroy_func clear_func)
{
    return (ptr_array_t*) array_new_full(sizeof(void*), size, clear_func);
}

void ptr_array_free(ptr_array_t* array)
{
    if (array != NULL) {
        if (array->array.data != NULL) {
            if (array->array.clear_func != NULL) {
                for (int i = 0; i < array->array.len; i++) {
                    array->array.clear_func(
                            *((void**) (array->array.data
                                    + i * array->array.elt_size)));
                }
            }
            free(array->array.data);
            array->array.data = NULL;
        }
        mem_free(array);
    }
}
ptr_array_t* ptr_array_remove_index(ptr_array_t* array, uint index)
{
    uint s;
    if (array == NULL)
        return NULL;
    if (array->array.data == NULL)
        return NULL;
    if (index >= array->array.len)
        return NULL;
    s = array->array.elt_size;
    if (array->array.clear_func != NULL)
        array->array.clear_func(*((void**) (array->array.data + s * index)));
    for (int i = index; i < array->array.len - 1; i++)
        memcpy(array->array.data + i * s, array->array.data + (i + 1) * s, s);
    array->array.len -= 1;
    return array;
}

ptr_array_t* ptr_array_remove_range(ptr_array_t* array, uint index, uint length)
{
    uint s;
    if (array == NULL)
        return NULL;
    if (array->array.data == NULL)
        return NULL;
    if ((index + length) > array->array.len)
        return NULL;
    if (length == 0)
        return NULL;
    s = array->array.elt_size;
    if (array->array.clear_func != NULL) {
        for (int i = index; i < index + length; i++)
            array->array.clear_func(*((void**) (array->array.data + s * i)));
    }
    for (int i = index; i < array->array.len; i++)
        memcpy(array->array.data + i * s, array->array.data + (i + length) * s,
                s);
    array->array.len -= length;
    return array;
}

ptr_array_t* ptr_array_append(ptr_array_t* array, const void *data)
{
    return (ptr_array_t*) array_append_val((array_t*) array, &(data));
}

ptr_array_t* ptr_array_prepend(ptr_array_t* array, const void *data)
{
    return (ptr_array_t*) array_prepend_val((array_t*) array, &(data));
}

ptr_array_t* ptr_array_insert(ptr_array_t* array, uint index, const void *data)
{
    return (ptr_array_t*) array_insert_val((array_t*) array, index, &(data));
}

void ptr_array_set(ptr_array_t* array, uint index, void *data)
{
    array_set((array_t*) array, index, &(data));
}

uint ptr_array_size(ptr_array_t* array)
{
    return array_size((array_t*) array);
}

void* ptr_array_at(ptr_array_t* array, uint index)
{
    return (*((void**) (array_at((array_t*) array, index))));
}

void ptr_array_remove_all(ptr_array_t *array)
{
    ptr_array_remove_range(array, 0, ptr_array_size(array));
}
/*
 *  str array
 */

str_array_t* str_array_new()
{
    return (str_array_t*) ptr_array_new();
}

str_array_t* str_array_new_sized(uint size)
{
    return (str_array_t*) ptr_array_new_sized(size);
}

str_array_t* str_array_new_full(uint size)
{
    return (str_array_t*) ptr_array_new_full(size, default_destroy_func);
}

void str_array_free(str_array_t* array)
{
    ptr_array_free((ptr_array_t*) array);
}

str_array_t* str_array_append(str_array_t* array, const char *data)
{
    return (str_array_t*) ptr_array_append((ptr_array_t*) array, data);
}

//如果待插入的字符串已经存在，则不插入，并返回false，否则返回true
bool str_array_append_unique(str_array_t* array, const char *data)
{
    for (int i = 0; i < str_array_size(array); i++) {
        if (strcmp(str_array_at(array, i), data) == 0) {
            return false;
        }
    }
    str_array_append(array, data);
    return true;
}

str_array_t* str_array_prepend(str_array_t* array, const char *data)
{
    return (str_array_t*) ptr_array_prepend((ptr_array_t*) array, data);
}

str_array_t* str_array_insert(str_array_t* array, uint index, const char *data)
{
    return (str_array_t*) ptr_array_insert((ptr_array_t*) array, index, data);
}

str_array_t* str_array_remove_index(str_array_t* array, uint index)
{
    return (str_array_t*) ptr_array_remove_index((ptr_array_t*) array, index);
}

bool str_array_remove(str_array_t* array, const char *data)
{
    for (int i = 0; i < str_array_size(array); i++) {
        if (strcmp(str_array_at(array, i), data) == 0) {
            str_array_remove_index(array, i);
            return true;
        }
    }
    return false;
}

str_array_t* str_array_remove_range(str_array_t* array, uint index, uint length)
{
    return (str_array_t*) ptr_array_remove_range((ptr_array_t*) array, index,
            length);
}

str_array_t* str_array_dup(str_array_t* array, destroy_func clear_func)
{
    str_array_t* new_array = (str_array_t*) array_duplicate((array_t*) array,
            clear_func);
    int array_len = str_array_size(array);
    for (int i = 0; i < array_len; i++) {
        str_array_set(new_array, i, strdup2(str_array_at(array, i)));
    }
    return new_array;
}

int str_array_find(str_array_t* array, char* data)
{
    for (int i = 0; i < str_array_size(array); i++) {
        if (strcmp(str_array_at(array, i), data) == 0) {
            return i;
        }
    }
    return -1;
}
uint str_array_size(str_array_t* array)
{
    return ptr_array_size((ptr_array_t*) array);
}
char* str_array_at(str_array_t* array, uint index)
{
    return (char*) ptr_array_at((ptr_array_t*) array, index);
}
void str_array_set(str_array_t* array, uint index, char* data)
{
    ptr_array_set((ptr_array_t*) array, index, data);
}
/*
 *  int array
 */
int_array_t* int_array_new()
{
    return (int_array_t*) array_new(sizeof(int));
}

int_array_t* int_array_new_sized(uint reserved_size)
{
    return (int_array_t*) array_new_sized(sizeof(int), reserved_size);
}

void int_array_free(int_array_t* array)
{
    array_free((array_t*) array);
}

int_array_t* int_array_append(int_array_t* array, int data)
{
    return (int_array_t*) array_append_val((array_t*) array, &data);
}

int_array_t* int_array_prepend(int_array_t* array, int data)
{
    return (int_array_t*) array_prepend_val((array_t*) array, &data);
}

int_array_t* int_array_insert(int_array_t* array, uint index, int data)
{
    return (int_array_t*) array_insert_val((array_t*) array, index, &data);
}

int_array_t* int_array_remove_index(int_array_t* array, uint index)
{
    return (int_array_t*) array_remove_index((array_t*) array, index);
}

bool int_array_remove(int_array_t* array, int data)
{
    for (int i = 0; i < int_array_size(array); i++) {
        if (int_array_at(array, i) == data) {
            int_array_remove_index(array, i);
            return true;
        }
    }
    return false;
}

int_array_t* int_array_remove_range(int_array_t* array, uint index, uint length)
{
    return (int_array_t*) array_remove_range((array_t*) array, index, length);
}
int_array_t* int_array_dup(int_array_t* array)
{
    return (int_array_t*) array_duplicate((array_t*) array, NULL);
}
int int_array_find(int_array_t* array, int data)
{
    for (int i = 0; i < int_array_size(array); i++) {
        if (int_array_at(array, i) == data) {
            return i;
        }
    }
    return -1;
}
uint int_array_size(int_array_t* array)
{
    return array_size((array_t*) array);
}
int int_array_at(int_array_t* array, uint index)
{
    return *((int*) array_at((array_t*) array, index));
}
void int_array_set(int_array_t* array, uint index, int data)
{
    array_set((array_t*) array, index, &data);
}

/*
 * *************************************
 */
void array_destroy_func(void* data)
{
    array_free((array_t*) data);
}

void ptr_array_destroy_func(void* data)
{
    ptr_array_free((ptr_array_t*) data);
}

void int_array_destroy_func(void* data)
{
    int_array_free((int_array_t*) data);
}

void str_array_destroy_func(void* data)
{
    str_array_free((str_array_t*) data);
}
