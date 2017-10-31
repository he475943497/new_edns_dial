/*
 * test_file.c
 *
 *  Created on: 2012-6-26
 *      Author: Administrator
 */

#include <clib/array.h>
#include <clib/socket.h>
#include <clib/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
void test_tcp_client()
{
    struct ip_addr local_ip;
    struct ip_addr remote_ip;
    in_addr_t ipv4;

    local_ip.version = IPV4;
//    ipv4 = inet_addr("192.168.20.128");
    local_ip.addr.ipv4 = 0;

    remote_ip.version = IPV4;
    ipv4 = inet_addr("127.0.0.1");
    remote_ip.addr.ipv4 = ipv4;

    int sock = tcp_new_client(NULL, 0, &remote_ip, 33333, 5, false);

    if (sock == -1) {
        LOG(LOG_LEVEL_TEST, "连接失败");
    }

    shutdown(sock,SHUT_WR);
}

void test_tcp_server()
{
    struct ip_addr ipv4;
    struct ip_addr ipv6;

    ipv4.version = IPV4;
    ipv4.addr.ipv4 = inet_addr("127.0.0.1");
//    ipv4.addr.ipv4 = 0;

    ipv6.version = IPV6;
//    ipv6.addr.ipv6 = in6addr_any;
    int ret = inet_pton(AF_INET6, "2001:da8:9000::82", &ipv6.addr.ipv6);
    if (ret <= 0) {
        LOG(LOG_LEVEL_TEST, "IP转换失败");
    }

    int sock;
    sock = tcp_new_server(&ipv6, 55, true);
    if (sock == -1) {
        LOG(LOG_LEVEL_TEST, "绑定失败");
    }
    sock = tcp_new_server(&ipv4, 55, true);
    if (sock == -1) {
        LOG(LOG_LEVEL_TEST, "绑定失败");
    }

    sleep(100);
}

void test_udp_server()
{
    uint count = 0;
    struct ip_addr ip;
    ip.version = IPV4;
    ip.addr.ipv4 = 0;
    LOG(LOG_LEVEL_TEST, "Start time:%d", time(NULL));
    while (count < 1245852141) {
        int fd = udp_new_socket(&ip, 0, true);
        if (fd == -1) {
            LOG_ERRNO(LOG_LEVEL_TEST, "NEW SOCKET FAIL!");
            continue;
        }
        close(fd);

        count++;
        if (count % 1000000 == 0) {
            LOG(LOG_LEVEL_TEST, "100万:%d", time(NULL));
        }
    }
}

void test_udp_system()
{
    struct ip_addr ip;
    ip.version = IPV4;
    ip.addr.ipv4 = 0;
    LOG(LOG_LEVEL_TEST, "Start time:%d", time(NULL));
    char *p=0;
    *p='2';
    int fd = tcp_new_server(&ip, 31256, true);
    if (fd == -1) {
        LOG_ERRNO(LOG_LEVEL_TEST, "NEW SOCKET FAIL!");
        return;
    }
//    system("/root/dnsys_3/DNSys/trunk/XForward/bin/xforward/xforward.py start");
    sleep(1000);
    close(fd);

}

void test_udp_server1()
{
    struct ip_addr ip;
    ip.version = IPV4;
    ip.addr.ipv4 = 0;
    LOG(LOG_LEVEL_TEST, "Start time:%d", time(NULL));

    int fd = udp_new_socket(&ip, 31256, true);
    if (fd == -1) {
        LOG_ERRNO(LOG_LEVEL_TEST, "NEW SOCKET FAIL!");
        return;
    }

    for (int i = 0; i < 2; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            LOG_ERRNO(LOG_LEVEL_ERROR, "main:fork error");
        } else if (pid == 0) { //子进程
            break;
        }
    }

    char buffer[65535];
    int count = 0;
    while (true) {
        struct sockaddr_in sockaddr4;
        socklen_t addr_len;
        int len = recvfrom(fd, buffer, sizeof(buffer), 0,
                (struct sockaddr *) &sockaddr4, &addr_len);
        if (len <= 0) {
            if (errno == EAGAIN) {
                continue;
            }
            LOG_ERRNO(LOG_LEVEL_ERROR, "recvfrom接收数据失败:%d", getpid());
            return;
        }
//        buffer[len] = '\0';

        usleep(1000);
        count++;
        if (count == 10000) {
            LOG(LOG_LEVEL_ERROR, "%d:10000", getpid());
            count -= 10000;
        }
    }
    close(fd);

}

void test_socket()
{
//    test_tcp_server();
    test_tcp_client();
//    test_udp_server1();
//    test_udp_system();
}
