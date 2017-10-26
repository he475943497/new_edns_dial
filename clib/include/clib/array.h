/**
 *Author:           wuyangchun,wanghonghui
 *Date:             2012-06-04
 *Description:      数组
 *
 *Notice:           ptr_array_t、 int_array_t、str_array_t都由array_t实现
 *                  封装起来一方面是为了便于调用
 *                  另一方面当看到ptr_array_t、int_array_t、str_array_t时，一眼就知道里面放的是什么类型的数据
 **/

#ifndef CLIB_ARRAY_H_
#define CLIB_ARRAY_H_

#include <clib/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _array array_t;          //通用数组
typedef struct _ptr_array ptr_array_t;      //存放指针的数组
typedef struct _int_array int_array_t;      //存放整形的数组
typedef struct _str_array str_array_t; //存放字符串的数组

/*
 *@brief        创建数组
 *@param in     element_size    数组内存放的元素的大小，
 *@param in     size            默认数组长度，如果存放的元素超过这个长度，数组自动按照这个长度来扩展
 *@param in     clear_func      释放数组时对数组内的每个元素掉用该函数，如果该参数为NULL，则释放数组时什么都不做
 *@return       新数组
 *@example      array_new(sizeof(int), 10, NULL);  创建一个存放int数据的数组,默认大小为10
 */
array_t* array_new(uint element_size);
array_t* array_new_sized(uint element_size, uint size);
array_t* array_new_full(uint element_size, uint size, destroy_func clear_func);

/*
 *@brief        释放数组
 *@param in     array       待释放数组
 *@return
 */
void array_free(array_t *array);

/*
 *@brief        往数组后面插入元素
 *@param in     array       待操作数组
 *@param in     data        待插入数据
 *@param in     len         数据个数
 *@return       调用时传入的array
 *@example      int a[3];   array_append_vals(int_array, a, 3);  往数组int_array中插入3个元素
 */
array_t* array_append_vals(array_t *array, const void *data, uint len);

/*
 *@brief        往数组前面插入元素
 *@param in     array       待操作数组
 *@param in     data        待插入数据
 *@param in     len         数据个数
 *@return       调用时传入的array
 */
array_t* array_prepend_vals(array_t *array, const void *data, uint len);

/*
 *@brief        往数组指定位置插入元素
 *@param in     array       待操作数组
 *@param in     index       插入元素位置，原来这个位置以及之后的元素都向后移动一位(index从0开始)
 *@param in     data        待插入数据
 *@param in     len         数据个数
 *@return       调用时传入的array
 */
array_t* array_insert_vals(array_t *array, uint index, const void *data,
        uint len);

/**
 * 操作单个数据
 */
/*
 *@brief        往数组后面插入单个元素
 *@param in     array       待操作数组
 *@param in     data        待插入数据的地址
 *@return       调用时传入的array
 *@example      int a;   array_append_vals(int_array, &a);  往数组int_array中插入3个元素
 */
array_t* array_append_val(array_t *array, const void *data);

/*
 *@brief        往数组前面插入单个元素
 *@param in     array       待操作数组
 *@param in     data        待插入数据的地址
 *@return       调用时传入的array
 */
array_t* array_prepend_val(array_t *array, const void *data);

/*
 *@brief        往数组指定位置插入单个元素
 *@param in     array       待操作数组
 *@param in     index       插入元素位置，原来这个位置以及之后的元素都向后移动一位(index从0开始)
 *@param in     data        待插入数据的地址
 *@return       调用时传入的array
 */

array_t* array_insert_val(array_t *array, uint index, const void *data);

/*
 *@brief        设置新的数组长度
 *@param in     array       待操作数组
 *@param in     length      新的长度，如果长度小于已有的长度，则什么都不做，否则扩充
 *@return       调用时传入的array
 */
array_t* array_set_size(array_t *array, uint length);

/*
 *@brief        删除指定位置的元素，如果创建数组时注册了清除函数，则先调用清除函数
 *@param in     array       待操作数组
 *@param in     index       待删除元素位置
 *@return       调用时传入的array
 */
array_t* array_remove_index(array_t *array, uint index);

/*
 *@brief        删除指定位置范围的元素，如果创建数组时注册了清除函数，则先调用清除函数
 *@param in     array       待操作数组
 *@param in     index       待删除元素起始位置(index从0开始)
 *@param in     length      待删除元素个数
 *@return       调用时传入的array
 */
array_t* array_remove_range(array_t *array, uint index, uint length);

/*
 *@brief        获取数组中元素个数
 *@return       数组中元素个数
 */
uint array_size(array_t *array);

/*
 *@brief        获取数组中指定位置的元素
 *@param in     array       待操作数组
 *@param in     index       元素位置
 *@return       指向数组中元素的指针
 */
void* array_at(array_t *array, uint index);

/*
 *@brief        设置数组中指定位置的元素
 *@param in     array       待操作数组
 *@param in     index       元素位置(index从0开始)
 *@param in     data        新的数据的地址
 *@return       调用时传入的array
 */
void array_set(array_t *array, uint index, void* data);

/*
 *@brief        复制数组
 *@param in     array       待复制数组
 *@return       全新的数组
 */
array_t* array_duplicate(array_t* array, destroy_func clear_func);

/**
 ******************************************************************************
 */
/**
 * 由上面的通用数组实现
 * 主要区别是：    1、指针数组里面存放的是指针，所以元素大小确定
 *              2、一般情况下不会出现批量插入，所以暂时不提供批量插入函数
 *              3、返回值为元素的值，也就是指针指向的位置，而不是元素的地址
 */
extern ptr_array_t* ptr_array_new();
extern ptr_array_t* ptr_array_new_sized(uint size);
extern ptr_array_t* ptr_array_new_full(uint size, destroy_func clear_func);

extern void ptr_array_free(ptr_array_t* array);

extern ptr_array_t* ptr_array_append(ptr_array_t* array, const void *data);

extern ptr_array_t* ptr_array_prepend(ptr_array_t* array, const void *data);
extern ptr_array_t* ptr_array_insert(ptr_array_t* array, uint index,
        const void *data);

extern ptr_array_t* ptr_array_remove_index(ptr_array_t* array, uint index);
extern ptr_array_t* ptr_array_remove_range(ptr_array_t* array, uint index,
        uint length);

extern uint ptr_array_size(ptr_array_t*  array);

extern void* ptr_array_at(ptr_array_t* array, uint index);
extern void ptr_array_set(ptr_array_t* array, uint index, void *data);
extern void ptr_array_remove_all(ptr_array_t *array);

/**
 ******************************************************************************
 */

/**
 * 针对字符串的实例化版本, 由于知道里面存放的是字符串，所以清除函数可以使用默认的清除函数
 */

//注意，只有通过str_array_new_full函数创建的数组，str_array_free的时候才会释放里面保存的字符串
extern str_array_t* str_array_new();
extern str_array_t* str_array_new_sized(uint size);
extern str_array_t* str_array_new_full(uint size);

extern void str_array_free(str_array_t* array);

extern str_array_t* str_array_append(str_array_t* array, const char *data);

//如果待插入的字符串已经存在，则不插入，并返回false，否则返回true
extern bool str_array_append_unique(str_array_t* array, const char *data);

extern str_array_t* str_array_prepend(str_array_t* array, const char *data);
extern str_array_t* str_array_insert(str_array_t* array, uint index,
        const char *data);
extern bool str_array_remove(str_array_t* array, const char *data);
extern str_array_t* str_array_remove_index(str_array_t* array, uint index);
extern str_array_t* str_array_remove_range(str_array_t* array, uint index,
        uint length);

extern str_array_t* str_array_dup(str_array_t* array, destroy_func clear_func);

extern int str_array_find(str_array_t* array, char* data);
extern uint str_array_size(str_array_t* array);
extern char* str_array_at(str_array_t* array, uint index);
extern void str_array_set(str_array_t* array, uint index, char* data);

/**
 ******************************************************************************
 */

/**
 * 针对整型的实例化版本, 由于知道里面存放的是int型，所以不需要清除函数
 */
extern int_array_t* int_array_new();
extern int_array_t* int_array_new_sized(uint reserved_size);
extern void int_array_free(int_array_t* array);

extern int_array_t* int_array_append(int_array_t* array, int data);
extern int_array_t* int_array_prepend(int_array_t* array, int data);
extern int_array_t* int_array_insert(int_array_t* array, uint index, int data);
extern int_array_t* int_array_remove_index(int_array_t* array, uint index);
extern bool int_array_remove(int_array_t* array, int data);
extern int_array_t* int_array_remove_range(int_array_t* array, uint index,
        uint length);

extern int_array_t* int_array_dup(int_array_t* array);
extern int int_array_find(int_array_t* array, int data);
extern uint int_array_size(int_array_t* array);
extern int int_array_at(int_array_t* array, uint index);
extern void int_array_set(int_array_t* array, uint index, int data);

//用来释放数组的destroy_func函数
extern void array_destroy_func(void* data);
extern void ptr_array_destroy_func(void* data);
extern void int_array_destroy_func(void* data);
extern void str_array_destroy_func(void* data);

#ifdef __cplusplus
}
#endif
#endif /* CLIB_ARRAY_H_ */
