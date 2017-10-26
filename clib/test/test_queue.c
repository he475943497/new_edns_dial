#include <pthread.h>
#include <clib/queue.h>
#include <clib/memory.h>
#include <clib/log.h>
#include <stdio.h>
#include <unistd.h>

volatile bool g_running;

static void* write_data1(void *arg)
{
    int counter = 0;
    int id = 0;
    queue_t *queue = (queue_t *) arg;
    char *array;
    while (g_running) {
        array = mem_alloc(32);
        sprintf(array, "aaaaaaaaaa:%d", id++);
        int count = queue_writen(queue, (void**) (&array), 1);
        if (count == 0) {
            LOG(LOG_LEVEL_TEST, "----------------------Buffer 已满");
            mem_free(array);
            usleep(100);
        }

        counter += count;
        if (counter >= 100000) {
            counter -= 100000;
            LOG(LOG_LEVEL_TEST, "write_data: 100000");
        }
    }
    return 0;
}

static void* write_data(void *arg)
{
    int counter = 0;
    int id = 0;
    queue_t *queue = (queue_t *) arg;
    char *array[2];
    while (g_running) {
        array[0] = mem_alloc(32);
        array[1] = mem_alloc(32);
        sprintf(array[0], "aaaaaaaaaa:%d", id++);
        sprintf(array[1], "aaaaaaaaaa:%d", id++);
        int count = queue_writen(queue, (void**) array, 2);
        if (count < 2) {
            LOG(LOG_LEVEL_TEST, "----------------------Buffer 已满");
            for (int i = count; i < 2; i++) {
                mem_free(array[i]);
            }
            usleep(100);
        }

        counter += count;
        if (counter >= 100000) {
            counter -= 100000;
            LOG(LOG_LEVEL_TEST, "write_data: 100000");
        }
    }
    return 0;
}

static void* read_data1(void *arg)
{
    queue_t *queue = (queue_t *) arg;
    int pid = pthread_self();
    char *array;
    int counter = 0;
    queue_reader_t *reader = queue_reader_new(queue);
    while (g_running) {
        int count = queue_readn(reader, 1, (void**) (&array));
        if (0 == count) {
//            LOG(LOG_LEVEL_TEST, "read_data1:--------------------Buffer 没数据");
            usleep(100);
        }

        counter += count;
        if (counter >= 100000) {
            LOG(LOG_LEVEL_TEST, "read_data1: 100000");
            counter -= 100000;
        }
//        for (int i = 0; i < count; i++) {
////            LOG(LOG_LEVEL_TEST, "%u:%s", pid, array[i]);
//        }

        queue_free_all(reader);

    }
    queue_reader_free(reader);
    return 0;
}

static void* read_data2(void *arg)
{
    queue_t *queue = (queue_t *) arg;
    int pid = pthread_self();
    void *array[2];
    int counter = 0;
    queue_reader_t *reader = queue_reader_new(queue);
    while (g_running) {
        int count = queue_readn(reader, 2, array);
        if (0 == count) {
//            LOG(LOG_LEVEL_TEST, "read_data2:--------------------Buffer 没数据");
            usleep(100);
        }
        counter += count;
        if (counter >= 100000) {
            LOG(LOG_LEVEL_TEST, "read_data2: 100000");
            counter -= 100000;
        }
//        for (int i = 0; i < count; i++) {
//            LOG(LOG_LEVEL_TEST, "%u:%s", pid, array[i]);
//        }

        queue_free_all(reader);

    }
    queue_reader_free(reader);
    return 0;
}

void test_queue()
{
    pthread_t thread_write;
    pthread_t thread_read1;
//    pthread_t thread_read2;

    g_running = true;

    queue_t *queue = queue_new(1000000, default_destroy_func);
    int ret = pthread_create(&thread_write, NULL, write_data1, queue);
    ret = pthread_create(&thread_read1, NULL, read_data1, queue);
//    ret = pthread_create(&thread_read2, NULL, read_data2, queue);

    sleep(300);
    g_running = false;

    pthread_join(thread_write, NULL);
    pthread_join(thread_read1, NULL);
//    pthread_join(thread_read2, NULL);

    queue_destroy(queue);

}
