#ifndef CLIB_IP_HASH_H
#define CLIB_IP_HASH_H

#include <clib/list.h>

/**
 *Author:           wanghonghui
 *Date:             2013-04-22
 *Description:      实现基于IP段的策略选择算法， 算法复杂度为O(1)
 **/

typedef struct {
    void *hash;
}ip_hash_t;

/*
 *@brief        IP段hash初始化函数
 *@param in     policy_num       支持的策略种类，目前支持最多16种
 *@return       创建的IP段hash数据结构         
 */
ip_hash_t * ip_hash_init();

/*
 *@brief        IP段hash释放函数
 *@param in     ip_hash ip_hash_init创建的IP段hash表          
 *@return                
 */
void ip_hash_uninit(ip_hash_t *ip_hash);

/*
 *@brief        IP段hash
 *@param in     ip_hash     IP段hash表
 *@param in     policy_idx  策略类型 必须小于创建时候指定的policy_num
 *@param in     start       IP段开始
 *@param in     end         IP段结束 
  *@param in    ptr         指向策略的指针，策略加入IP段hash表后就不应该再修改，并保持所占用的内存。
 *@return                   成功返回 0， 失败返回-1
 */
int ip_hash_add(ip_hash_t *ip_hash, unsigned int start, unsigned int end, void *ptr);

/*
 *@brief        IP段hash
 *@param in     ip_hash     IP段hash表
 *@param in     policy_idx  策略类型 必须小于创建时候指定的policy_num
 *@param in     start       IP段开始
 *@return       ptr         函数只想成功后，ptr指向具体的策略       
 *@return                   成功返回 0， 失败返回-1
 */
list_t *ip_hash_search(ip_hash_t *ip_hash, unsigned int ip);

#endif
