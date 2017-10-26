#ifndef CLIB_IP_HASH_H
#define CLIB_IP_HASH_H

#include <clib/list.h>

/**
 *Author:           wanghonghui
 *Date:             2013-04-22
 *Description:      ʵ�ֻ���IP�εĲ���ѡ���㷨�� �㷨���Ӷ�ΪO(1)
 **/

typedef struct {
    void *hash;
}ip_hash_t;

/*
 *@brief        IP��hash��ʼ������
 *@param in     policy_num       ֧�ֵĲ������࣬Ŀǰ֧�����16��
 *@return       ������IP��hash���ݽṹ         
 */
ip_hash_t * ip_hash_init();

/*
 *@brief        IP��hash�ͷź���
 *@param in     ip_hash ip_hash_init������IP��hash��          
 *@return                
 */
void ip_hash_uninit(ip_hash_t *ip_hash);

/*
 *@brief        IP��hash
 *@param in     ip_hash     IP��hash��
 *@param in     policy_idx  �������� ����С�ڴ���ʱ��ָ����policy_num
 *@param in     start       IP�ο�ʼ
 *@param in     end         IP�ν��� 
  *@param in    ptr         ָ����Ե�ָ�룬���Լ���IP��hash���Ͳ�Ӧ�����޸ģ���������ռ�õ��ڴ档
 *@return                   �ɹ����� 0�� ʧ�ܷ���-1
 */
int ip_hash_add(ip_hash_t *ip_hash, unsigned int start, unsigned int end, void *ptr);

/*
 *@brief        IP��hash
 *@param in     ip_hash     IP��hash��
 *@param in     policy_idx  �������� ����С�ڴ���ʱ��ָ����policy_num
 *@param in     start       IP�ο�ʼ
 *@return       ptr         ����ֻ��ɹ���ptrָ�����Ĳ���       
 *@return                   �ɹ����� 0�� ʧ�ܷ���-1
 */
list_t *ip_hash_search(ip_hash_t *ip_hash, unsigned int ip);

#endif
