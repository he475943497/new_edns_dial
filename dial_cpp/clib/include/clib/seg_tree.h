/**
 *Author:           吴阳春
 *Date:             2012-06-07
 *Description:      区间树，内部由二叉树实现
 *                  以范围段为KEY，插入的时候如果KEY和已有数据重叠，则自动拆分为多个KEY，保证KEY范围不重叠
 *
 *Notice：                          目前只支持无符号的整形范围
 **/

#ifndef CLIB_RANGE_TREE_H
#define CLIB_RANGE_TREE_H

#include <clib/types.h>
#include <clib/list.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _seg_tree seg_tree_t;


/*
 *@brief        区间树遍历函数指针
 *@param in     start       区间开始值
 *@param in     end         区间结束值
 *@param in     value       数据列表
 *@param in     user_data   用户自定义数据，遍历时由调用程序传入
 *@return       返回值的意义由具体的场景定义
 */
typedef bool (*seg_traverse_func)(uint start, uint end, list_t* value, void* user_data);

/*
 *@brief        创建区间树
 *@param in     data_destroy_func  当从set_tree中移除一条数据时，负责释放由data指向的内存
 *@return       新区间树
 */
extern seg_tree_t* seg_tree_new(destroy_func data_destroy_func);

/*
 *@brief        向区间树中插入数据
 *@param in     tree    区间树
 *@param in     start   区间开始值
 *@param in     end     区间结束值
 *@param in     data    新的数据
 *@return
 */
void seg_tree_insert(seg_tree_t *seg_tree, uint start, uint end, void* data);

/*
 *@brief        根据key查找对应的data
 *@param in     tree            区间树
 *@param in     key             待查找的key
 *@return       如果key落在树中的某个区间，返回data列表，否则返回NULL
 */
extern list_t* seg_tree_lookup(seg_tree_t *tree, const uint key);

/*
 *@brief        释放区间树
 *@param in     tree    待释放的区间树
 *@return
 */
extern void seg_tree_free(seg_tree_t *tree);

/*
 *@brief        遍历区间树，如果遍历函数返回true，则停止遍历
 *@param in     tree            区间树
 *@param in     func            遍历函数
 *@param in     user_data       用户自定义数据，调用遍历函数的时候会传过去
 *@return
 */
extern void seg_tree_foreach(seg_tree_t *tree, seg_traverse_func func, void* user_data);


#ifdef __cplusplus
}
#endif

#endif
