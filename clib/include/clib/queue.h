/**
 *Author:           wuyangchun
 *Date:             2012-09-21
 *Description:      队列（多线程安全），支持一个线程写，多个线程读
 *                  保证多个线程能按照同样的顺序读到同样的数据
 *
 **/

#ifndef CLIB_QUEUE_H_
#define CLIB_QUEUE_H_

#include <clib/types.h>

typedef struct _queue queue_t;
typedef struct _queue_reader queue_reader_t;

/*
 *@brief        创建队列
 *@param in     queue_size          队列大小，超过该大小后，数据将放不进去
 *@param in     data_destory_func   存放数据的释放函数
 *@return       -1失败，其他成功
 *
 */
extern queue_t* queue_new(uint queue_size, destroy_func data_destory_func);

/*
 *@brief        删除队列
 *@param in     queue          待删除队列
 *
 */
extern void queue_destroy(queue_t* queue);

/*
 *@brief        获取队列的读取对象
 *@param in     queue               将要读取的队列
 *@return       文件读取对象，利用该对象可以读取队列的数据
 *
 */
extern queue_reader_t *queue_reader_new(queue_t* queue);

/*
 *@brief        删除队列的读取对象
 *@param in     reader               待删除的读取对象
 *
 */
extern void queue_reader_free(queue_reader_t* reader);

/*
 *@brief        将数据写入队列
 *@param in     queue
 *@param in     data        数据数组
 *@param in     count       数据个数
 *@return       实际写入的个数，如果队列空间不足，写入的个数可能小于要写入的个数
 *
 */
extern int queue_writen(queue_t* queue, void **data, int count);

/*
 *@brief        从数据队列中获取数据
 *@param in     queue
 *@param in     count       要读取的数据个数
 *@param out    p           返回的数据
 *@return       实际读取的数据个数，如果队列个数小于要读的数据个数，返回值小于count
 *
 */
extern int queue_readn(queue_reader_t* queue_reader, int count, void **p);

/*
 *@brief        释放reader对象已经读取的所有数据
 *@param in     queue_reader
 *@notice       通过reader读到数据，在使用完之后，需要调用该函数来释放
 *
 */
extern void queue_free_all(queue_reader_t* queue_reader);

#endif /* CLIB_QUEUE_H_ */
