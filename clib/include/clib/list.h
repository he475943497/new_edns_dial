/*
 * list.c
 *
 *  Created on: 2012-5-29
 *      Author: Administrator
 */
#ifndef CLIB_LIST_H_
#define CLIB_LIST_H_
	 
#include <clib/types.h>
	 
#ifdef __cplusplus
	 extern "C" {
#endif 
	 
 typedef struct _list list_t;
 
 struct list_node {
	 void *data;
	 struct list_node *next;
	 struct list_node *prev;
 };
 






#include <clib/list.h>
#include <clib/memory.h>
#include <clib/memcache.h>

struct _list {
    destroy_func free_func;
    uint length;
    struct list_node *head;
    struct list_node *tail;
    mem_cache_t* mem_cache;
    uint cache_size;
};

static list_t* list_new(uint size, destroy_func func)
{
    struct _list* list = (struct _list*) mem_alloc(sizeof(struct _list));

    list->length = 0;
    list->free_func = func;
    list->head = NULL;
    list->tail = NULL;
    list->mem_cache = mem_cache_create(sizeof(struct list_node), size);
    list->cache_size = size;

    return list;
}



static struct list_node* list_append(list_t *list, void *data)
{
    struct list_node *new_node;

    new_node = mem_cache_alloc(list->mem_cache);
    new_node->data = data;
    new_node->next = NULL;

    if (list->tail != NULL) {
        list->tail->next = new_node;
        new_node->prev = list->tail;
        list->tail = new_node;
    } else {
        new_node->prev = NULL;
        list->head = new_node;
        list->tail = new_node;
    }

    list->length++;

    return new_node;
}

static struct list_node* list_append_unique(list_t *list, void *data,
        compare_func func, void *user_data)
{
    struct list_node *temp_node;

    temp_node = list->head;
    while (temp_node != NULL) {
        if (func(data, temp_node->data, user_data) == 0) { //找到了
            if (list->free_func != NULL) {
                list->free_func(data);
            }
            return temp_node;
        }
        temp_node = temp_node->next;
    }

    return list_append(list, data);
}

static struct list_node* list_prepend(list_t *list, void *data)
{
    struct list_node *new_node;

    new_node = mem_cache_alloc(list->mem_cache);
    new_node->data = data;
    new_node->prev = NULL;

    new_node->next = list->head;
    list->head = new_node;

    if (list->tail == NULL) {
        list->tail = new_node;
    }

    list->length++;

    return new_node;
}

static struct list_node* list_insert_before(list_t *list, struct list_node *sibling,
        void *data)
{
    struct list_node *new_node;

    new_node = mem_cache_alloc(list->mem_cache);
    new_node->data = data;
    if (sibling->prev != NULL) {
        sibling->prev->next = new_node;
    } else { //sibling is head
        list->head = new_node;
    }

    new_node->prev = sibling->prev;
    new_node->next = sibling;
    sibling->prev = new_node;

    list->length++;

    return new_node;
}


static struct list_node* list_insert_sorted(list_t *list, void *data,
        compare_func func, void *user_data)
{
    struct list_node *temp_node;

    temp_node = list->head;
    while (temp_node != NULL) {
        if (func(data, temp_node->data, user_data) > 0) {
            return list_insert_before(list, temp_node, data);
        }
        temp_node = temp_node->next;
    }

    return list_append(list, data);
}


static struct list_node* list_insert_after(list_t *list, struct list_node *sibling,
        void *data)
{
    struct list_node *new_node;

    new_node = mem_cache_alloc(list->mem_cache);
    new_node->data = data;

    if (sibling->next != NULL) {
        sibling->next->prev = new_node;
    } else {//sibling is tail
        list->tail = new_node;
    }
    new_node->prev = sibling;
    new_node->next = sibling->next;
    sibling->next = new_node;

    list->length++;

    return new_node;
}

static void list_concat(list_t *list1, list_t *list2)
{
    if (list2->head == NULL) { //list2为空
        return;
    }

    if (list1->head == NULL) {  //list1为空
        list1->head = list2->head;
        list1->tail = list2->tail;
        list1->length = list2->length;

    } else {
        list1->tail->next = list2->head;
        list2->head->prev = list1->tail;
        list1->tail = list2->tail;
        list1->length += list2->length;
    }

    list2->head = NULL;
    list2->tail = NULL;

}

static void list_copy(list_t *list1, list_t *list2)
{
    if(list1 == NULL || list2 == NULL){
        return;
    }
    if (list2->head == NULL) { //list2为空
        return;
    }

    struct list_node *nd = list2->head;
    struct list_node *new_nd;
    while(nd){
        new_nd = mem_cache_alloc(list1->mem_cache);
        new_nd->data = nd->data;
        new_nd->next = NULL;        
        new_nd->prev = NULL;        
        if(list1->tail == NULL){
            list1->head = list1->tail = new_nd;
        }else{
            new_nd->prev = list1->tail;
            list1->tail->next = new_nd;
            list1->tail = new_nd;
        }
        list1->length++;
        nd = nd->next;
    }

}

static void list_remove_node(list_t *list, struct list_node *node)
{
    if (node == NULL) {
        return;
    }

    if (node->prev == NULL && node->next == NULL) {
        //empty
        list->head = NULL;
        list->tail = NULL;
    } else if (node->prev == NULL) { //remove head
        node->next->prev = NULL;
        list->head = node->next;
    } else if (node->next == NULL) { //remove tail
        node->prev->next = node->next;
        list->tail = node->prev;
    } else {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }
    if (list->free_func != NULL) {
        list->free_func(node->data);
    }
    mem_cache_free(list->mem_cache, node);
    list->length--;

}


static bool list_remove(list_t *list, const void *data, compare_func func,
        void *user_data)
{
    struct list_node *temp_node;

    temp_node = list->head;
    while (temp_node != NULL) {
        if (func(data, temp_node->data, user_data) == 0) {
            list_remove_node(list, temp_node);
            return true;
        }
        temp_node = temp_node->next;
    }

    return false;
}

//static void list_remove_node_internal(list_t *list, struct list_node *node, bool steal)
//{
//    if (node->prev == NULL && node->next == NULL) {
//        //empty
//    } else if (node->prev == NULL) {
//        node->next->prev = NULL;
//    } else if (node->next == NULL) {
//        node->prev->next = node->next;
//    } else {
//        node->prev->next = node->next;
//        node->next->prev = node->prev;
//    }
//
//    list->free_func(node->data);
//    mem_free(node);
//    list->length--;
//
//}


static void list_remove_all(list_t *list)
{
    struct list_node *traverse_node;
    struct list_node *temp_node;

    traverse_node = list->head;
    while (traverse_node != NULL) {
        temp_node = traverse_node->next;
        if (list->free_func != NULL) {
            list->free_func(traverse_node->data);
        }
        mem_cache_free(list->mem_cache, traverse_node);
        list->length--;
        traverse_node = temp_node;
    }
    list->head = NULL;
    list->tail = NULL;
}

/*
 *@brief        释放列表
 *@param in     list      待释放列表
 *@return
 */
static void list_free(list_t *list)
{
    list_remove_all(list);
    mem_cache_destroy(list->mem_cache);
    mem_free(list);
}

//struct list_node * list_steal_data(list_t *list, const void *data,
//        compare_func func)
//{
//    struct list_node *temp_node;
//
//    temp_node = list->head;
//    while (temp_node != NULL) {
//        if (func(data, temp_node->data) == 0) {
//            if (temp_node->prev == NULL && temp_node->next == NULL) {
//                //empty
//            } else if (temp_node->prev == NULL) {
//                temp_node->next->prev = NULL;
//            } else if (temp_node->next == NULL) {
//                temp_node->prev->next = temp_node->next;
//            } else {
//                temp_node->prev->next = temp_node->next;
//                temp_node->next->prev = temp_node->prev;
//            }
//            list->length--;
//
//            return temp_node;
//        }
//        temp_node = temp_node->next;
//    }
//
//    return NULL;
//}

/*
 *@brief        从列表中获取一个节点
 *              该节点会从列表中删除，但不会调用destroy_func
 *@param in     list        待操作列表
 *@param in     node        待获取的节点
 *@return       获取到的节点, 等于参数中传入的节点
 */
//struct list_node * list_steal(list_t *list, struct list_node *node);
static void list_reverse(list_t *list)
{
    struct list_node *temp_node;

    if (list->head == NULL || list->length == 1) {
        return;
    }

    temp_node = list->head;
    list->head = list->tail;
    list->tail = temp_node;
}

/*
 *@brief        拷贝列表
 *@param in     list        待拷贝列表
 *@return       拷贝出来的新的列表
 */
static list_t* list_duplicate(const list_t *list, destroy_func func)
{
    list_t* new_list = (list_t*) mem_alloc(sizeof(struct _list));
    struct list_node *temp_node;

    new_list->length = list->length;
    new_list->free_func = func;
    new_list->mem_cache = mem_cache_create(sizeof(struct list_node),
            list->cache_size);
    new_list->cache_size = list->cache_size;
    new_list->head = NULL;
    new_list->tail = NULL;

    temp_node = list->head;
    while (temp_node != NULL) {
        list_append(new_list, temp_node->data);
        temp_node = temp_node->next;
    }

    return new_list;
}

static struct list_node* list_find(list_t *list, const void *data, compare_func func,
        void *user_data)
{
    struct list_node *temp_node;

    temp_node = list->head;
    while (temp_node != NULL) {
        if (func(data, temp_node->data, user_data) == 0) {
            return temp_node;
        }
        temp_node = temp_node->next;
    }

    return NULL;
}

static struct list_node * list_tail(list_t *list)
{
    return list->tail;
}

static struct list_node * list_head(list_t *list)
{
    return list->head;
}

static uint list_size(list_t *list)
{
    return list->length;
}

static void list_foreach(list_t *list, traverse_func func, void *user_data)
{
    struct list_node *temp_node;

    RETURN_IF_FAIL(list != NULL);
    RETURN_IF_FAIL(func != NULL);

    temp_node = list->head;
    while (temp_node != NULL) {
        if (func(temp_node->data, user_data) == true) {
            break;
        }
        temp_node = temp_node->next;
    }
}

/*
 *@brief        排序
 *@param in     list        列表
 *@param in     func        比较函数
 *@param in     asc         tree/false：升序/降序
 *@return
 */
 
//void list_sort(list_t *list, compare_func func, bool asc);
static void list_destroy_func(void* data)
{
    list_free((list_t*) data);
}

#ifdef __cplusplus
}
#endif 

#endif /* CLIB_LIST_H_ */

