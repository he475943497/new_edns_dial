/*
 * udp.c
 *
 *  Created on: 2012-11-8
 *      Author: yamu
 */
#include <clib/udp.h>
#include <strings.h>
#include <string.h>
#include <clib/log.h>
#include <unistd.h>
#include <clib/socket.h>

int udp_sendto(int fd, char* data, uint data_len, const struct ip_addr* ip, uint16_t port)
{
    struct sockaddr_in sockaddr4;
    struct sockaddr_in6 sockaddr6;
    struct sockaddr *addr = NULL;
    socklen_t addr_len;
    if (ip->version == IPV4) {
        bzero(&sockaddr4, sizeof(sockaddr4));
        sockaddr4.sin_family = AF_INET;
        sockaddr4.sin_port = htons(port);
        memcpy(&sockaddr4.sin_addr, &ip->addr.ipv4, sizeof(sockaddr4));
        addr = (struct sockaddr *) &sockaddr4;
        addr_len = sizeof(sockaddr4);
    } else {
        bzero(&sockaddr6, sizeof(sockaddr6));
        sockaddr6.sin6_family = AF_INET6;
        sockaddr6.sin6_port = htons(port);
        memcpy(&sockaddr6.sin6_addr, &ip->addr.ipv6, sizeof(sockaddr6.sin6_addr));
        addr = (struct sockaddr *) &sockaddr6;
        addr_len = sizeof(sockaddr6);

    }

    return sendto(fd, data, data_len, 0, addr, addr_len);
}

int udp_new_socket(const struct ip_addr *ip, uint16_t port,
        bool nonblock)
{
    int sockfd = socket(ip->version == IPV4 ? AF_INET : AF_INET6, SOCK_DGRAM,
            0);
    if (sockfd == -1) {
        LOG_ERRNO(LOG_LEVEL_ERROR, "创建udp socket失败");
        return -1;
    }

    int optval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *) &optval,
            sizeof(int)) < 0) {
        LOG_ERRNO(LOG_LEVEL_ERROR, "设置REUSEADDR失败");
        close(sockfd);
        return -1;
    }

    struct sockaddr_in sockaddr4;
    struct sockaddr_in6 sockaddr6;
    struct sockaddr *addr;
    socklen_t len;

    if (ip->version == IPV4) {
        bzero(&sockaddr4, sizeof(sockaddr4));
        sockaddr4.sin_family = AF_INET;
        sockaddr4.sin_port = htons(port);
        sockaddr4.sin_addr.s_addr = ip->addr.ipv4;
        addr = (struct sockaddr *) &sockaddr4;
        len = sizeof(sockaddr4);
    } else {
        bzero(&sockaddr6, sizeof(sockaddr6));
        sockaddr6.sin6_family = AF_INET6;
        sockaddr6.sin6_port = htons(port);
        memcpy(&sockaddr6.sin6_addr, &ip->addr.ipv6, sizeof(ip->addr.ipv6));
        addr = (struct sockaddr *) &sockaddr6;
        len = sizeof(sockaddr6);
    }

    int ret = bind(sockfd, addr, len);
    if (ret == -1) {
        LOG_ERRNO(LOG_LEVEL_ERROR, "绑定失败(%d)", port);
        close(sockfd);
        return -1;
    }

    if (nonblock) {
        if (!socket_set_blocking(sockfd, false)) {
            close(sockfd);
            return -1;
        }
    }

    return sockfd;
}
