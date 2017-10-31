/**
 *Author:           wuyangchun
 *Date:             2012-06-05
 *Description:      哈希表， 大小不够的时候会自动扩展，但数据被移除的时候不会自动缩小，需要手动调用，
                                                     这样可以避免添加和删除的时候刚好在自动扩展的边界上，导致不停的调整哈希表大小
 *
 **/

#ifndef CLIB_HASH_H_
#define CLIB_HASH_H_

#include <clib/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _hash_table hash_table_t;

typedef struct _hash_table_iter hash_table_iter;


/*
 *@brief        创建哈希表
 *@param in     size                默认大小，如果超过这个大小，每次增加这么多个，如果为0，采取默认策略
 *@param in     hash_func           根据key得到哈希值的函数
 *@param in     key_compare_func    判断key是否相等的函数
 *@param        user_data           用户自定义数据，会传入给key_compare_func
 *@param in     key_destroy_func    当从哈希表中移除一条数据时，负责释放由key指向的内存
 *@param in     value_destroy_func  当从哈希表中移除一条数据时，负责释放由value指向的内存
 *@return       新哈希表
 */
hash_table_t* hash_table_new(/*uint size,*/ hash_func hash_func,
        compare_func key_compare_func, void *user_data, destroy_func key_destroy_func,
        destroy_func value_destroy_func);

/*
 *@brief        释放哈希表
 *@param in     hash_table      待释放哈希表
 *@return
 */
void hash_table_free(hash_table_t *hash_table);

/*
 *@brief        向哈希表插入数据,如果key已经存在，替换掉value值
                                            如果创建hash_table的时候指定了value_destroy_func，则调用该函数释放原来的value
 *@param in     hash_table      哈希表
 *@param in     key             待插入的key
 *@param in     value           待插入的数据
 *@return
 */
void hash_table_insert(hash_table_t *hash_table, void *key, void *value);

/*
 *@brief        替换key对应的value,如果key不存在，插入新的数据
 *@param in     hash_table      哈希表
 *@param in     key             待替换value的key
 *@param in     value           新的value
 *@return
 */
//void hash_table_replace(hash_table_t *hash_table, void *key, void *value);

/*
 *@brief        删除key及关联的value
 *@param in     hash_table      哈希表
 *@param in     key             删除的key
 *@return       如果key不存在，返回false，否则返回true
 */
bool hash_table_remove(hash_table_t *hash_table, const void *key);

/*
 *@brief        删除哈希表内所有数据，同时调用hash_table_resize
 *@param in     hash_table      哈希表
 *@return
 */
void hash_table_remove_all(hash_table_t *hash_table);

/*
 *@brief        获取key对应的数据,并从哈希表中移除相应的数据
 *              但不调用key和value的destroy_func
 *@param in     hash_table      哈希表
 *@param in     lookup_key      待查找的key
 *@param out    orikey          存放找到的key，如果不需要保存返回值，传入NULL即可
 *@param out    value           存放找到的value，如果不需要保存返回值，传入NULL即可
 *@return       如果key存在，返回true，否则返回false
 */
bool hash_table_steal(hash_table_t *hash_table,
        const void *lookup_key, void **orikey, void **value);

/*
 *@brief        根据key查找对应的value
 *@param in     hash_table      哈希表
 *@param in     key             待查找的key
 *@return       如果key存在，返回value，否则返回NULL
 *
 *@notice       如果value本身的值就是NULL，该函数无法区分key是否存在
 *              这种情况下需要调用hash_table_lookup_extended函数
 */
void *hash_table_lookup(hash_table_t *hash_table, const void *key);

/*
 *@brief        判断key是否存在
 *@param in     hash_table      哈希表
 *@param in     key             待查找的key
 *@return       如果key存在，返回true，否则返回false
 */
bool hash_table_contains(hash_table_t *hash_table, const void *key);

/*
 *@brief        根据key查找对应的value
 *@param in     hash_table      哈希表
 *@param in     lookup_key      待查找的key
 *@param out    orikey          存放找到的key，如果不需要，传入NULL即可
 *@param out    value           存放找到的value，如果不需要，传入NULL即可
 *@return       如果key存在，返回true，否则返回false
 */
bool hash_table_lookup_extended(hash_table_t *hash_table,
        const void *lookup_key, void **orikey, void **value);

/*
 *@brief        遍历哈希表，如果遍历函数返回true，则停止遍历
 *@param in     hash_table      哈希表
 *@param in     func            遍历函数
 *@param in     user_data       用户自定义数据，调用遍历函数的时候会传过去
 *@return       如果全部遍历完成，返回true，否则返回false
 */
bool hash_table_foreach(hash_table_t *hash_table, traverse_pair_func func,
        void *user_data);

/*
 *@brief        遍历哈希表，如果遍历函数返回true，则删除掉对应的key和value
 *@param in     hash_table      哈希表
 *@param in     func            遍历函数
 *@param in     user_data       用户自定义数据，调用遍历函数的时候会传过去
 *@return       实际删除的数据个数
 */
uint hash_table_foreach_remove(hash_table_t *hash_table, traverse_pair_func func,
        void *user_data);

/*
 *@brief        获取哈希表大小（存放的数据个数）
 *@param in     hash_table      哈希表
 *@return       哈希表大小
 */
uint hash_table_size(hash_table_t *hash_table);

/*
 *@brief        根据哈希表的实际大小，重新调整哈希表的占用空间，释放多余的空间
 *@param in     hash_table      哈希表
 *@return
 */
void hash_table_resize(hash_table_t *hash_table);

/*
 *@brief        获取哈希表的key列表
 *@param in     hash_table      哈希表
 *@return       key列表，包含所有的key
 */
//list_t* hash_table_get_keys(hash_table_t *hash_table);

/*
 *@brief        获取哈希表的value列表
 *@param in     hash_table      哈希表
 *@return       value列表，包含所有的value
 */
//list_t* hash_table_get_values(hash_table_t *hash_table);

/*
 *@brief        初始化哈希表的迭代器
 *              初始化后调用hash_table_iter_next就可以获得哈希表的第一个数据
 *@param in     iter            迭代器
 *@param in     hash_table      哈希表
 *@return
 */
//void hash_table_iter_init(hash_table_iter *iter, hash_table_t *hash_table);

/*
 *@brief        取哈希表中的下一个数据
 *@param in     iter        迭代器
 *@param out    key         取到的key
 *@param out    value       取到的value
 *@return
 */
//bool hash_table_iter_next(hash_table_iter *iter, void **key, void **value);

/*
 *@brief        根据迭代器得到关联的哈希表
 *@param in     iter        迭代器
 *@return       关联的哈希表，如果还没有关联哈希表，则返回NULL
 */
//hash_table_t* hash_table_iter_get_hash_table(hash_table_iter *iter);

/*
 *@brief        移除迭代器指向的数据
 *@param in     iter        迭代器
 *@return
 */
//void hash_table_iter_remove(hash_table_iter *iter);

/*
 *@brief        用新的value替换掉迭代器指向的value
 *@param in     iter        迭代器
 *@param in     value       新的value
 *@return
 */
//void hash_table_iter_replace(hash_table_iter *iter, void *value);



/* 哈希函数
 */
uint str_hash(const void *v);
uint int32_hash(const void *v);
uint int_hash(const void *v);
uint int64_hash(const void *v);
uint double_hash(const void *v);
uint direct_hash(const void *v);
uint ip_hash(const void *v);

//用来释放哈希表的destroy_func函数
extern void hash_table_destroy_func(void* data);

#ifdef __cplusplus
}
#endif
#endif /* CLIB_HASH_H_ */
