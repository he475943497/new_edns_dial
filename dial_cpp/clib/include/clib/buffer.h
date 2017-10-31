/**
 *Author:           wuyangchun
 *Date:             2012-07-26
 *Description:      buffer
 *
 *Notice:           目前主要用于发包的时候构造数据包
 **/

#ifndef CLIB_BUFFER_H_
#define CLIB_BUFFER_H_

#include <clib/types.h>

typedef struct _buffer buffer_t;

/*
 *@brief        新建一个buffer
 *@param in     reserve_size    buffer占用空间的长度，如果使用过程中长度超过这个长度，则按照这个长度扩展
 *@return
 */
buffer_t* buffer_new(uint reserve_size);

/*
 *@brief        新建一个buffer，在buffer前面保留指定空间，后面所有append的数据都放在保留空间的后面
 *              buffer创建好之后，buffer的长度就等于保留空间大小的长度
 *@param in     reserve_size    buffer占用空间的长度，如果使用过程中长度超过这个长度，则按照这个长度扩展
 *@param in     unused_size     保留空间大小
 *@return       如果unused_size > reserve_size, 返回NULL
 */
buffer_t* buffer_new_unused(uint reserve_size, uint unused_size);

/*
 *@brief        向buffer后面添加数据
 *@param in     buffer
 *@param in     data        数据地址
 *@param in     len         数据长度
 *@return
 */
void buffer_append(buffer_t* buffer, const void* data, uint len);

/*
 *@brief        向buffer里面添加整形数据
 *@param in     buffer
 *@param in     data        整形数据
 *@return
 */
void buffer_append_int8(buffer_t* buffer, int8_t data);
void buffer_append_int16(buffer_t* buffer, int16_t data);
void buffer_append_int32(buffer_t* buffer, int32_t data);
void buffer_append_int64(buffer_t* buffer, int64_t data);

/*
 *@brief        向目的buffer里面追加一个源buffer
 *@param in     buffer_dst      目的buffer
 *@param in     buffer_src      源buffer
 *@return
 */
void buffer_append_buffer(buffer_t* buffer_dst, const buffer_t* buffer_src);


/*
 *@brief        清空buffer里面的数据
                                          如果buffer是通过buffer_new_unused创建的，则继续在前面保留指定的空间
 *@param in     buffer      待清空的buffer
 *@return
 */
void buffer_clean(buffer_t* buffer);

/*
 *@brief        释放buffer及里面包含的数据
 *@param in     buffer      目的buffer
 *@return
 */
void buffer_free(buffer_t* buffer);

char* buffer_data(buffer_t* buffer);

uint buffer_length(buffer_t* buffer);

#endif /* CLIB_BUFFER_H_ */
