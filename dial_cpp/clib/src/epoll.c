#include <clib/epoll.h>
#include <clib/memory.h>
#include <sys/epoll.h>
#include <errno.h>
#include <clib/log.h>
#include <unistd.h>
#include <clib/memcache.h>

struct _epoll {
    int epfd;
    int socket_count;
    mem_cache_t* mem_cache;
};

struct _event_data {
    int socket;
    epoll_callback_func callback;
    void *user_data;
    uint8_t ip_version;
};

epoll_t* epoll_new(int socket_count)
{
    int epfd;
    if ((epfd = epoll_create(socket_count)) == -1) {
        LOG_ERRNO(LOG_LEVEL_WARNING, "epoll create");
        return NULL;
    }

    struct _epoll* epoll = (struct _epoll*) mem_alloc(sizeof( struct _epoll));
    epoll->epfd = epfd;
    epoll->socket_count = socket_count;
    epoll->mem_cache = mem_cache_create(sizeof(struct _event_data),
            socket_count > 1024 ? 1024 : socket_count);

    return epoll;
}

int epoll_add(epoll_t* epoll, uint8_t ip_version, int socket, int event,
        epoll_callback_func callback, void *user_data)
{
    struct _epoll* _epoll = (struct _epoll*) epoll;

    struct epoll_event ev;
    ev.events = event;

    struct _event_data* event_data = mem_cache_alloc(_epoll->mem_cache);
    ev.data.ptr = event_data;
    event_data->socket = socket;
    event_data->callback = callback;
    event_data->user_data = user_data;
    event_data->ip_version = ip_version;

    int ret = epoll_ctl(_epoll->epfd, EPOLL_CTL_ADD, socket, &ev);
    if (ret < 0) {
        LOG_ERRNO(LOG_LEVEL_WARNING, "epoll_ctl");
        mem_cache_free(_epoll->mem_cache, event_data);
        return -1;
    }

    return 0;
}

int epoll_remove(epoll_t* epoll, int socket)
{
    struct _epoll* _epoll = (struct _epoll*) epoll;

    struct epoll_event ev;

    int ret = epoll_ctl(_epoll->epfd, EPOLL_CTL_DEL, socket, &ev);
    if (ret < 0) {
        LOG_ERRNO(LOG_LEVEL_WARNING, "epoll_ctl");
        return -1;
    }
    mem_cache_free(_epoll->mem_cache, ev.data.ptr);

    return 0;
}

int epoll_start(epoll_t* epoll, volatile bool* running)
{
    int ret = 0;
    struct _epoll* _epoll = (struct _epoll*) epoll;
    struct epoll_event *events =
            mem_alloc(sizeof(struct epoll_event)*_epoll->socket_count);
    while (*running) {
        ret = 0;
        int fds = epoll_wait(_epoll->epfd, events, _epoll->socket_count, 1*1000);
        if (ret < 0) {
            if (errno == EINTR) { //信号中断
                continue;
            }
            LOG_ERRNO(LOG_LEVEL_WARNING, "epoll_wait");
            break;
        }
        for (int i = 0; i < fds; i++) {
            struct _event_data* event_data = (struct _event_data*)events[i].data.ptr;
            ret = event_data->callback(event_data->ip_version, event_data->socket, event_data->user_data);
            if (ret < 0) {
                epoll_remove(epoll, event_data->socket);
                close(event_data->socket);
            }
        }
    }

    mem_free(events);

    return ret;
}

int epoll_free(epoll_t *epoll)
{
    struct _epoll* _epoll = (struct _epoll*) epoll;

    mem_cache_destroy(_epoll->mem_cache);
    close(_epoll->epfd);

    mem_free(_epoll);

    return 0;
}
