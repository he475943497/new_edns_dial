/*
 * buffer.c
 *
 *  Created on: 2012-7-20
 *      Author: Administrator
 */
#include <clib/buffer.h>
#include <clib/memory.h>
#include <string.h>

#define DEFAULT_RESERVE_SIZE 10

struct _buffer {
    char *data;         //数据
    uint len;           //长度
    uint reserve_size;  //空间长度
    uint extend_size;   //每次扩展的大小
    uint unused_size;   //buffer前面保留的空间长度
};

buffer_t * buffer_new(uint reserve_size)
{
    if (reserve_size <= 0) {
        reserve_size = DEFAULT_RESERVE_SIZE;
    }
    struct _buffer *_buf = (struct _buffer *) mem_alloc(sizeof(struct _buffer));

    _buf->len = 0;
    _buf->reserve_size = reserve_size;
    _buf->data = malloc(reserve_size);   //由于后面有relloc，所以不调用mem_alloc
    _buf->extend_size = reserve_size;
    _buf->unused_size = 0;

    return (buffer_t *) _buf;
}

buffer_t* buffer_new_unused(uint reserve_size, uint unused_size)
{
    if(unused_size > reserve_size){
        return NULL;
    }
    buffer_t * buffer = buffer_new(reserve_size);
    struct _buffer *_buf = (struct _buffer *) buffer;
    _buf->len = unused_size;
    _buf->unused_size = unused_size;

    return buffer;
}

void buffer_clean(buffer_t* buffer)
{
    struct _buffer *_buf = (struct _buffer *) buffer;
    _buf->len = _buf->unused_size;
}

static void extend_size(buffer_t* buf, uint step)
{
    struct _buffer *_buf = (struct _buffer *) buf;
    _buf->data = realloc(_buf->data,
            _buf->reserve_size + _buf->extend_size * step);
}

void buffer_append(buffer_t* buffer, const void* data, uint len)
{
    struct _buffer *_buf = (struct _buffer *) buffer;
    if (_buf->len + len > _buf->reserve_size) {
        extend_size(buffer,
                (_buf->len + len - _buf->reserve_size - 1) / _buf->extend_size
                        + 1);
    }
    memcpy(_buf->data + _buf->len, data, len);
    _buf->len += len;
}

void buffer_append_int8(buffer_t* buffer, int8_t data)
{
    buffer_append(buffer, &data, sizeof(data));
}
void buffer_append_int16(buffer_t* buffer, int16_t data)
{
    buffer_append(buffer, &data, sizeof(data));
}
void buffer_append_int32(buffer_t* buffer, int32_t data)
{
    buffer_append(buffer, &data, sizeof(data));
}
void buffer_append_int64(buffer_t* buffer, int64_t data)
{
    buffer_append(buffer, &data, sizeof(data));
}

void buffer_append_buffer(buffer_t* buffer_dst,
        const buffer_t* buffer_src)
{
    buffer_append(buffer_dst, buffer_src->data, buffer_src->len);
}

void buffer_free(buffer_t* buffer)
{
    free(buffer->data);
    mem_free(buffer );
}


char* buffer_data(buffer_t* buffer)
{
    struct _buffer *_buf = (struct _buffer *) buffer;
    return _buf->data;
}

uint buffer_length(buffer_t* buffer)
{
    struct _buffer *_buf = (struct _buffer *) buffer;
    return _buf->len;
}
