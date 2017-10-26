/**
 *Author:           wuyangchun
 *Date:             2012-06-06
 *Description:      平衡二叉树
 *
 **/
#ifndef CLIB_TREE_H_
#define CLIB_TREE_H_

#include <clib/types.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef struct _tree_t tree_t;

/*
 *@brief        创建二叉树
 *@param in     size                默认节点个数，如果超过这个个数，每次增加这么多个，如果为0，采取默认策略
 *@param in     key_compare_func    判断key是否相等的函数
 *@param in     key_destroy_func    当从tree中移除一条数据时，负责释放由key指向的内存
 *@param in     value_destroy_func  当从tree中移除一条数据时，负责释放由value指向的内存
 *@return       新二叉树
 */
tree_t* tree_new(uint size, compare_func key_compare_func, void *user_data);
tree_t* tree_new_full(uint size, compare_func key_compare_func, void *user_data,
        destroy_func key_destroy_func, destroy_func value_destroy_func);

/*
 *@brief        释放二叉树
 *@param in     tree    待释放的二叉树
 *@return
 */
void tree_free(tree_t *tree);

/*
 *@brief        向二叉树中插入数据, 如果key已存在，则更新value
 *@param in     tree    二叉树
 *@param in     key     新的key
 *@param in     value   新的数据
 *@return
 */
void tree_insert(tree_t *tree, void *key, void *value);

/*
 *@brief        替换二叉树中key对应的数据，如果key不存在，则插入新的数据
 *@param in     tree    二叉树
 *@param in     key     key
 *@param in     value   新的数据
 *@return
 */
//void tree_replace(tree_t *tree, void *key, void *value);

/*
 *@brief        从二叉树中移除指定的key以及关联的value
 *@param in     tree    二叉树
 *@param in     key     待删除的key
 *@return       如果key不存在，返回false，否则返回true
 */
bool tree_remove(tree_t *tree, const void *key);
/*
 * @brief       remove all item in tree
 * @param in    tree, tobe clean
 */
void tree_remove_all(tree_t *tree);
/*
 *@brief        从二叉树中移除第一个节点，既最左边一个叶子节点
 *@param in     tree    二叉树
 *@return       如果树为空或者删除之后树为空，返回NULL，否则返回删除之后，树的第一个节点里的key
 */
void * tree_remove_first_node(tree_t *tree);

/*
 *@brief        从二叉树中移除最后一个节点，既最右边一个叶子节点
 *@param in     tree    二叉树
 *@return       如果树为空或者删除之后树为空，返回NULL，否则返回删除之后，树的最后一个节点里的key
 */
void * tree_remove_last_node(tree_t *tree);

/*
 *@brief        获取key对应的数据,并从tree中移除相应的数据
 *              但不调用key和value的destroy_func
 *@param in     tree            二叉树
 *@param in     lookup_key      待查找的key
 *@param out    orikey          存放找到的key，如果不需要保存返回值，传入NULL即可
 *@param out    value           存放找到的value，如果不需要保存返回值，传入NULL即可
 *@return       如果key存在，返回true，否则返回false
 */
bool tree_steal(tree_t *tree, const void *lookup_key, void **orikey,
        void **value);

/*
 *@brief        根据key查找对应的value
 *@param in     tree            二叉树
 *@param in     key             待查找的key
 *@return       如果key存在，返回value，否则返回NULL
 *
 *@notice       如果value本身的值就是NULL，该函数无法区分key是否存在
 *              这种情况下需要调用tree_lookup_extended函数
 */
void *tree_lookup(tree_t *tree, const void *key);

/*
 *@brief        根据key查找对应的value
 *@param in     tree            二叉树
 *@param in     lookup_key      待查找的key
 *@param out    orikey          存放找到的key, 如果不需要，传入NULL即可
 *@param out    value           存放找到的value, 如果不需要，传入NULL即可
 *@return       如果key存在，返回true，否则返回false
 */
bool tree_lookup_extended(tree_t *tree, const void *lookup_key, void **orikey,
        void **value);

/*
 *@brief        升序遍历二叉树，如果遍历函数返回true，则停止遍历
 *@param in     tree            二叉树
 *@param in     func            遍历函数
 *@param in     user_data       用户自定义数据，调用遍历函数的时候会传过去
 *@return
 */
void tree_foreach(tree_t *tree, traverse_pair_func func, void *user_data);

/*
 *@brief        降序遍历二叉树，如果遍历函数返回true，则停止遍历
 *@param in     tree            二叉树
 *@param in     func            遍历函数
 *@param in     user_data       用户自定义数据，调用遍历函数的时候会传过去
 *@return
 */
void tree_reverse_foreach(tree_t *tree, traverse_pair_func func, void *user_data);

/*
 *@brief        返回二叉树的高度
 *@param in     tree            二叉树
 *@return       二叉树的高度
 */
uint tree_height(tree_t *tree);

/*
 *@brief        返回二叉树的节点数
 *@param in     tree            二叉树
 *@return       二叉树的节点数
 */
uint tree_nnodes(tree_t *tree);

//用来释放列表的destroy_func函数
extern void tree_destroy_func(void* data);


#ifdef __cplusplus
}
#endif

#endif /* CLIB_TREE_H_ */
