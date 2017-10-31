/**
 *Author:           wuyangchun
 *Date:             2012-06-01
 *Description:
 *
 *Notice:
 **/


#ifndef CLIB_TYPES_H_
#define CLIB_TYPES_H_

#include <clib/macros.h>
#include <stdint.h>
#include <stddef.h>

#ifndef _STDBOOL_H
#define _STDBOOL_H
#ifdef __cplusplus
extern "C" {
#endif

#ifndef __cplusplus
#define false (0)
#define true (!false)
typedef int bool;
#endif 
#endif


#ifndef  UINT
typedef unsigned int uint;
#define UINT
#endif

/*
 *@brief        比较函数指针,比较a，b的大小
 *@param in     a       第一个数据
 *@param in     b       第二个数据
 *@return       大于0：    a > b
 *              等于0：    a == b
 *              小于0：    a < b
 */
typedef int (*compare_func)(const void *a, const void *b, void *user_data);

/*
 *@brief        hash函数指针
 *@param in     key     需要计算hash值的数据
 *@return       根据key计算出来的hash值
 *
 */
typedef uint (*hash_func)(const void *key);

/*
 *@brief        数据释放函数指针
 *@param in     data     待释放的函数
 *@return
 *
 */
typedef void (*destroy_func)(void *data);

/*
 *@brief        遍历函数指针，用来遍历列表、数组等一个节点只有一个数据的结构
 *@param in     data        当前数据
 *@param in     user_data   遍历时用到的数据
 *@return       返回值的意义由具体的场景定义
 *
 */
typedef bool (*traverse_func)(void *data, void *user_data);

/*
 *@brief        遍历函数指针，用来遍历key、value这种结构的数据
 *@param in     key         当前key
 *@param in     value       当前value
 *@param in     user_data   遍历时用到的数据
 *@return       返回值的意义由具体的场景定义
 *
 */
typedef bool (*traverse_pair_func)(void *key, void *value, void *user_data);

//默认数据释放函数
extern void default_destroy_func(void* data);

//已经实现的几种常见数据类型的比较函数
extern int int32_compare_func(const void *a, const void *b, void *user_data);
extern int uint32_compare_func(const void *a, const void *b, void *user_data);
extern int int_compare_func(const void *a, const void *b, void *user_data);
extern int uint_compare_func(const void *a, const void *b, void *user_data);
extern int str_compare_func(const void *a, const void *b, void *user_data);
extern int double_compare_func(const void *a, const void *b, void *user_data);
extern int int64_compare_func(const void *a, const void *b, void *user_data);
extern int direct_compare_func(const void *a, const void *b, void *user_data);
extern int ip_compare_func(const void *a, const void *b, void *user_data);


#ifdef __cplusplus
}
#endif

#endif /* CLIB_TYPES_H_ */
