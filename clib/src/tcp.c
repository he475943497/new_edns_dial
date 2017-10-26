/*
 * tcp.c
 *
 *  Created on: 2012-11-8
 *      Author: yamu
 */
#include <clib/tcp.h>
#include <strings.h>
#include <string.h>
#include <clib/log.h>
#include <unistd.h>
#include <clib/socket.h>
#include <errno.h>



int  tcp_new_client(const struct ip_addr *local_ip, uint16_t local_port,
        const struct ip_addr *server_ip, uint16_t server_port, time_t timeout,
        bool nonblock)
{
    if(local_ip != NULL && local_ip->version != server_ip->version){
        LOG_ERRNO(LOG_LEVEL_ERROR, "源IP和目的IP的版本不一致");
        return -1;
    }
    int sockfd = socket(server_ip->version == IPV4 ? AF_INET : AF_INET6,
            SOCK_STREAM, 0);
    if (sockfd == -1) {
        LOG_ERRNO(LOG_LEVEL_ERROR, "创建tcp socket失败");
        return -1;
    }

    struct sockaddr_in server_sockaddr4;
    struct sockaddr_in6 server_sockaddr6;
    struct sockaddr *server_addr;
    socklen_t server_addr_len;

    struct sockaddr_in local_sockaddr4;
    struct sockaddr_in6 local_sockaddr6;
    struct sockaddr *local_addr;
    socklen_t local_addr_len;

    if (server_ip->version == IPV4) {
        bzero(&local_sockaddr4, sizeof(local_sockaddr4));
        local_sockaddr4.sin_family = AF_INET;
        local_sockaddr4.sin_port = htons(local_port);
        if (local_ip != NULL) {
            local_sockaddr4.sin_addr.s_addr = local_ip->addr.ipv4;
        }
        local_addr = (struct sockaddr *) &local_addr;
        local_addr_len = sizeof(local_sockaddr4);

        bzero(&server_sockaddr4, sizeof(server_sockaddr4));
        server_sockaddr4.sin_family = AF_INET;
        server_sockaddr4.sin_port = htons(server_port);
        server_sockaddr4.sin_addr.s_addr = server_ip->addr.ipv4;
        server_addr = (struct sockaddr *) &server_sockaddr4;
        server_addr_len = sizeof(server_sockaddr4);
    } else {
        bzero(&local_sockaddr6, sizeof(local_sockaddr6));
        local_sockaddr6.sin6_family = AF_INET6;
        local_sockaddr6.sin6_port = htons(local_port);
        if (local_ip != NULL) {
            memcpy(&local_sockaddr6.sin6_addr, &local_ip->addr.ipv6,
                    sizeof(local_ip->addr.ipv6));
        }
        local_addr = (struct sockaddr *) &local_sockaddr6;
        local_addr_len = sizeof(local_sockaddr6);

        bzero(&server_sockaddr6, sizeof(server_sockaddr6));
        server_sockaddr6.sin6_family = AF_INET6;
        server_sockaddr6.sin6_port = htons(server_port);
        memcpy(&server_sockaddr6.sin6_addr, &server_ip->addr.ipv6,
                sizeof(server_ip->addr.ipv6));
        server_addr = (struct sockaddr *) &server_sockaddr6;
        server_addr_len = sizeof(server_sockaddr6);
    }
//    retvalue = inet_pton(AF_INET, ip, &sockaddr.sin_addr);
//    LOG_RET_ERR_IF_ZERO(retvalue, LOG_LEVEL_ERROR, "IP地址格式错误：%s", ip);

    if (timeout >= 0) {
        if (!socket_set_blocking(sockfd, false)) {
            close(sockfd);
            return -1;
        }
    }

    if (local_ip != NULL && local_port != 0) {
        int ret = bind(sockfd, local_addr, local_addr_len);
        if (ret == -1) {
            LOG_ERRNO(LOG_LEVEL_ERROR, "bind socket失败");
            close(sockfd);
            return -1;
        }
    }

    int ret = connect(sockfd, server_addr, server_addr_len);
    if (ret == -1) {
        if (errno != EINPROGRESS) {
            LOG_ERRNO(LOG_LEVEL_ERROR, "连接服务器失败");
            close(sockfd);
            return -1;
        }
        struct timeval tm;
        fd_set set;
        tm.tv_sec = timeout;
        tm.tv_usec = 0;
        FD_ZERO(&set);
        FD_SET(sockfd, &set);
        int ret_select = select(sockfd + 1, NULL, &set, NULL, &tm);
        if (ret_select > 0) {
            int error_no = -1;
            int len = sizeof(error_no);
            int ret_opt = getsockopt(sockfd, SOL_SOCKET, SO_ERROR,
                    (char*) &error_no, (socklen_t *) &len);
            if (ret_opt == -1) {
                LOG_ERRNO(LOG_LEVEL_ERROR, "getsockopt失败");
                close(sockfd);
                return -1;
            } else {
                if (error_no != 0) {
                    char errno_str[1024];
                    strerror_r(error_no, errno_str, sizeof(errno_str));
                    errno_str[sizeof(errno_str) - 1] = '\0';
                    LOG(LOG_LEVEL_ERROR, "连接失败:%s", errno_str);
                    close(sockfd);
                    return -1;
                }
            }
        } else if (ret_select == 0) { //timeout
            LOG(LOG_LEVEL_ERROR, "连接超时");
            close(sockfd);
            return -1;
        } else //error
        {
            if (errno == EINPROGRESS) {
                LOG(LOG_LEVEL_ERROR, "连接超时");
            } else {
                LOG_ERRNO(LOG_LEVEL_ERROR, "getsockopt失败");
            }
        }
    }

    if (!nonblock) {
        if (!socket_set_blocking(sockfd, true)) {
            close(sockfd);
            return -1;
        }
    }

    return sockfd;
}


/*
 *@brief        创建一个tcp socket，并绑定到指定IP和端口
 *@param in     ip          绑定的服务器IP(网络序)
 *@param in     port        绑定的服务器端口(主机序)
 *@param in     nonblock    是否阻塞模式
 *@return       -1失败，否则成功
 */
int  tcp_new_server(const struct ip_addr *ip, uint16_t port,
        bool nonblock)
{
    int sockfd = socket(ip->version == IPV4 ? AF_INET : AF_INET6, SOCK_STREAM,
            0);
    if (sockfd == -1) {
        LOG_ERRNO(LOG_LEVEL_ERROR, "创建tcp socket失败");
        return -1;
    }

    if (nonblock) {
        if (!socket_set_blocking(sockfd, false)) {
            close(sockfd);
            return -1;
        }
    }

    int optval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *) &optval,
            sizeof(int)) < 0) {
        LOG_ERRNO(LOG_LEVEL_ERROR, "设置REUSEADDR失败");
        close(sockfd);
        return -1;
    }

    if (ip->version == IPV6) {
        int yes = 1;
        if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, (void *) &yes,
                sizeof(yes)) < 0) {
            LOG_ERRNO(LOG_LEVEL_ERROR, "设置IPV6_V6ONLY no失败");
            close(sockfd);
            return -1;
        }
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

    //    retvalue = inet_pton(AF_INET, ip, &sockaddr.sin_addr);
    //    LOG_RET_ERR_IF_ZERO(retvalue, LOG_LEVEL_ERROR, "IP地址格式错误：%s", ip);

    int ret = bind(sockfd, addr, len);
    if (ret == -1) {
        LOG_ERRNO(LOG_LEVEL_ERROR, "绑定失败(%d)", port);
        close(sockfd);
        return -1;
    }

    ret = listen(sockfd, 5);
    if (ret == -1) {
        LOG_ERRNO(LOG_LEVEL_ERROR, "监听失败");
        close(sockfd);
        return -1;
    }

    return sockfd;
}


int  tcp_send(int fd, const char* data, uint len, int timeout)
{
    int send_len = 0;
    int temp_send_len = 0;
    struct timeval tv;

    if (timeout > 0) {
        tv.tv_sec = timeout; /* 30 Secs Timeout */
        tv.tv_usec = 0;  // Not init'ing this can cause strange errors

        setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *) &tv,
                sizeof(struct timeval));
    }

    while (send_len < len) {
        temp_send_len = send(fd, data + send_len, len - send_len, 0);
        if (temp_send_len == -1) {
            if (errno == EAGAIN) {
                LOG(LOG_LEVEL_ERROR, "发送数据超时或者被中断");
                return -1;
            }
            LOG_ERRNO(LOG_LEVEL_ERROR, "发送报文失败");
            return -1;
        }
        send_len += temp_send_len;
    }

    return 0;
}

int tcp_recv(int fd, char* recv_buf, uint buf_len, int timeout)
{
    int read_len = 0;
    int temp_read_len = 0;
    struct timeval tv;

    if (timeout > 0) {
        tv.tv_sec = timeout; /* 30 Secs Timeout */
        tv.tv_usec = 0;  // Not init'ing this can cause strange errors

        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *) &tv,
                sizeof(struct timeval));
    }

    while (read_len < buf_len) {
        temp_read_len = recv(fd, recv_buf + read_len, buf_len - read_len, 0);
        if (temp_read_len == -1) {
            if (errno == EAGAIN) { //Non-blocking I/O has been selected using O_NONBLOCK and no data was immediately available for reading
                LOG(LOG_LEVEL_ERROR, "接收数据超时或者被中断");
                return -1;
            }
            LOG_ERRNO(LOG_LEVEL_ERROR, "接收报文失败");
            return -1;
        }
        if (temp_read_len == 0) {
            LOG_ERRNO(LOG_LEVEL_ERROR, "连接已断开");
            return -1;
        }
        read_len += temp_read_len;
    }

    return 0;
}
