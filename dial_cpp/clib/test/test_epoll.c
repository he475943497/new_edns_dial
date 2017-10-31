/*
 * test_epoll.c
 *
 *  Created on: 2012-7-30
 *      Author: Administrator
 */
#include <clib/epoll.h>
#include <clib/log.h>
#include <clib/ip.h>
#include <clib/socket.h>
#include <sys/epoll.h>
#include<signal.h>

#define MAX_EPOLL_FD  65535u

int test(uint8_t ip_version, int socket, void *user_data)
{
    LOG(LOG_LEVEL_TEST, "get a connection");
    return 0;
}

volatile bool g_running = true;

static void ctrl_c_op(int signo)
{
    LOG(LOG_LEVEL_ERROR, "ctrl+c");
    g_running = false;
}


void test_epoll()
{

    //初始化信号处理
    struct sigaction act;
    act.sa_handler = ctrl_c_op;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if (sigaction(SIGINT, &act, NULL) < 0) {
        LOG_ERRNO(LOG_LEVEL_ERROR, "注册信号处理函数失败");
        return ;
    }

    epoll_t *epoll = NULL;
    if ((epoll = epoll_new(MAX_EPOLL_FD)) == NULL) {
        LOG_ERRNO(LOG_LEVEL_ERROR, "epoll create");
        return;
    }

    struct ip_addr local_ip;
    local_ip.version = IPV4;
//    ipv4 = inet_addr("192.168.20.128");
    local_ip.addr.ipv4 = 0;

    int bindfd = tcp_new_server(&local_ip, 53, true);

    int ret = epoll_add(epoll, IPV4, bindfd, EPOLLIN, // | EPOLLET,
            (epoll_callback_func) test, NULL);

    LOG_RET_IF_ERR(ret, LOG_LEVEL_ERROR, "epoll_add");

    ret = epoll_start(epoll, &g_running);
    LOG_IF_ERR(ret, LOG_LEVEL_ERROR, "epoll_start");

    epoll_free(epoll);
}
