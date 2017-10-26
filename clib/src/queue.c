#include <clib/queue.h>
#include <clib/list.h>
#include <clib/memory.h>
#include <clib/memcache.h>
#include <clib/list.h>
#include <clib/log.h>
#include <pthread.h>
#include <string.h>
#include <clib/array.h>

struct _queue {
    destroy_func data_destroy_func;
    uint8_t ref_count;              //reader ref数量
    pthread_rwlock_t rwlock;
    uint max_size;
    uint reserve_size;
    uint size;
    array_t *array;
    uint start;
    uint end;
};

struct _queue_node {
    void *data;         //这个字段一定要放在结构体的最前面
    uint8_t ref_count; //ref数量，每取一次+1，释放一次-1
};

struct _queue_reader {
    struct _queue *queue;
    uint start;
    uint end;
};

queue_t* queue_new(uint size, destroy_func data_destory_func)
{
    struct _queue *queue;

    if (size == 0)
        return NULL;

    queue = (struct _queue *) mem_alloc(sizeof(struct _queue));
    if (queue == NULL)
        return NULL;
    queue->data_destroy_func = data_destory_func;
    pthread_rwlock_init(&(queue->rwlock), NULL);
    queue->ref_count = 0;

    queue->start = 0;
    queue->end = 0;
    queue->max_size = size; //空一个，保证有数据的时候start != end
    queue->reserve_size = size + 1; //空一个，保证有数据的时候start != end
    queue->size = 0;

    queue->array = array_new_sized(sizeof(struct _queue_node),
            queue->reserve_size);
    struct _queue_node temp;
    bzero(&temp, sizeof(temp));
    for (uint i = 0; i < queue->reserve_size; i++) {
        array_append_val(queue->array, &temp);
    }

    return queue;
}

void queue_destroy(queue_t* queue)
{
    if (queue != NULL) {
        pthread_rwlock_destroy(&(queue->rwlock));
        if (queue->data_destroy_func != NULL) {
            for (uint i = queue->start; i != queue->end;) {
                struct _queue_node * p = (struct _queue_node *) array_at(
                        queue->array, i);
                queue->data_destroy_func(p->data);

                i++;
                if (i == queue->reserve_size) {
                    i -= queue->reserve_size;
                }
            }
        }
        array_free(queue->array);
        mem_free(queue);
    }
}

queue_reader_t *queue_reader_new(queue_t* queue)
{
    queue_reader_t *p;
    if (queue == NULL)
        return NULL;
    p = mem_alloc(sizeof(struct _queue_reader));
    if (p == NULL)
        return NULL;
    pthread_rwlock_wrlock(&(queue->rwlock));
    p->queue = queue;
    queue->ref_count++;
    p->start = queue->start;
    p->end = queue->start;
    pthread_rwlock_unlock(&(queue->rwlock));
    return p;
}

void queue_reader_free(queue_reader_t* reader)
{
    if (reader->start != reader->end) {
        queue_free_all(reader);
    }
    queue_t *queue = reader->queue;
    pthread_rwlock_wrlock(&(queue->rwlock));
    int index = reader->end;
    while (queue->size-- > 0) {
        if (index == queue->end) {
            break;
        }
        struct _queue_node * p = (struct _queue_node *) array_at(queue->array,
                index++);
        if (p->ref_count + 1 == queue->ref_count) {
            if (queue->data_destroy_func != NULL) {
                queue->data_destroy_func(p->data);
            }
            queue->start++;
            queue->size--;
            if (queue->start == queue->reserve_size) {
                queue->start -= queue->reserve_size;
                index -= queue->reserve_size;
            }
        } else {
            break;
        }
    }
    pthread_rwlock_unlock(&(queue->rwlock));
    mem_free(reader);
}

int queue_writen(queue_t* queue, void **data, int count)
{
    int write_count;
    int i;
    if (queue == NULL && data == NULL) {
        return 0;
    }

    pthread_rwlock_wrlock(&(queue->rwlock));
    if (queue->max_size == queue->size) { //满了
        pthread_rwlock_unlock(&(queue->rwlock));
        return 0;
    }

    if (queue->max_size < queue->size + count) {
        write_count = queue->max_size - queue->size;
    } else {
        write_count = count;
    }

    for (i = 0; i < write_count; i++) {
        struct _queue_node *node = array_at(queue->array, queue->end);
        node->data = data[i];
        node->ref_count = 0;

        queue->end++;
        queue->size++;
        if (queue->end == queue->reserve_size) {
            queue->end -= queue->reserve_size;
        }
    }
    pthread_rwlock_unlock(&(queue->rwlock));
    return i;
}

int queue_readn(queue_reader_t* queue_reader, int count, void **data)
{

    int i;
    if (queue_reader == NULL || count == 0 || data == NULL) {
        return 0;
    }
    queue_t *queue = queue_reader->queue;

//    if (queue->size == 0) {
//        return 0;
//    }

    pthread_rwlock_rdlock(&(queue->rwlock));
    if (queue->size == 0) {
        pthread_rwlock_unlock(&(queue->rwlock));
        return 0;
    }

    for (i = 0; i < count; i++) {
        if (queue_reader->end == queue->end) {
            pthread_rwlock_unlock(&(queue->rwlock));
            return i;
        }
        struct _queue_node* node = (struct _queue_node*) array_at(queue->array,
                queue_reader->end);
        data[i] = node->data;
        queue_reader->end++;
        if (queue_reader->end == queue->reserve_size) {
            queue_reader->end -= queue->reserve_size;
        }
    }
    pthread_rwlock_unlock(&(queue->rwlock));

    return count;
}

void queue_free_all(queue_reader_t* queue_reader)
{
    struct _queue_node *p;
    queue_t *queue = queue_reader->queue;
    if (queue == NULL)
        return;
    pthread_rwlock_wrlock(&(queue->rwlock));
    while (queue_reader->start != queue_reader->end) {
        p = (struct _queue_node*) array_at(queue->array, queue_reader->start);
        p->ref_count++;
        queue_reader->start++;
        if (queue_reader->start == queue->reserve_size) {
            queue_reader->start -= queue->reserve_size;
        }
        if (p->ref_count == queue->ref_count) {
            if (queue->data_destroy_func != NULL) {
                queue->data_destroy_func(p->data);
            }
            queue->start++;
            queue->size--;
            if (queue->start == queue->reserve_size) {
                queue->start -= queue->reserve_size;
            }

            if (queue_reader->start != queue->start) {
                LOG(LOG_LEVEL_ERROR, "reader->start:%d, queue->start:%d",
                        queue_reader->start, queue->start);
                exit(-1);
            }
        }
    }
    pthread_rwlock_unlock(&(queue->rwlock));
    return;
}
