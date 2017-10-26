#include <clib/socket.h>
#include <netpacket/packet.h>
#include <linux/if.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <clib/log.h>
#include <string.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
//#include <netdb.h>
//#include <netinet/in.h>

bool socket_set_blocking(int sockfd, bool blocking)
{
    unsigned long ul = blocking ? 0 : 1;
    int ret_ioctl = ioctl(sockfd, FIONBIO, &ul, sizeof(ul)); //set Non-blocking
    if (ret_ioctl == -1) {
        LOG_ERRNO(LOG_LEVEL_ERROR, "设置阻塞模式失败");
        return false;
    };

    return true;
}


int socket_new_raw_server(const char *eth, uint recv_buf, bool nonblock)
{
    struct ifreq ifr;
    struct sockaddr_ll sll;
    int ret = 0;
    //ETH_P_ALL,ETH_P_IPV6
    int sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd == -1) {
        LOG_ERRNO(LOG_LEVEL_ERROR, "创建raw socket失败");
        return -1;
    }

    if (recv_buf > 0) {
        int buffer_size = recv_buf * 1024;
        ret = setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (const char*) &buffer_size,
                sizeof(int));
        if (ret == -1) {
            LOG_ERRNO(LOG_LEVEL_WARNING, "设置缓冲区大小失败:%d(k)", recv_buf);
        }
    }

    if (eth != NULL && strlen(eth) > 0) {
        memset(&ifr, 0, sizeof(ifr));
        strcpy(ifr.ifr_name, eth);
        if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1) {
            LOG_ERRNO(LOG_LEVEL_WARNING, "根据网口名称获取索引失败:%s", eth);
            close(sockfd);
            return -1;
        }
        memset(&sll, 0, sizeof(sll));
        sll.sll_family = AF_PACKET;
        sll.sll_ifindex = ifr.ifr_ifindex;
        sll.sll_protocol = htons(ETH_P_IP);
        if (bind(sockfd, (struct sockaddr *) (&sll), sizeof(sll)) == -1) {
            LOG_ERRNO(LOG_LEVEL_ERROR, "绑定网口失败%d", ifr.ifr_ifindex);
            close(sockfd);
            return -1;
        }
    }

    if (nonblock) {
        if (!socket_set_blocking(sockfd, false)) {
            close(sockfd);
            return -1;
        }
    }

    return sockfd;
}



int socket_set_recv_buf(int fd, uint size)
{
    uint buffer_size = size * 1024;
    int ret = setsockopt(fd, SOL_SOCKET, SO_RCVBUF,
            (const char*) &buffer_size, sizeof(int));
    if (ret == -1) {
        LOG_ERRNO(LOG_LEVEL_WARNING, "设置接收缓冲区大小失败:%d(k)",
                size);
        return -1;
    }

    buffer_size = 0;
    socklen_t buffer_size_len = sizeof(buffer_size);
    ret = getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &buffer_size,
            &buffer_size_len);
    if (ret == -1) {
        LOG_ERRNO(LOG_LEVEL_WARNING, "获取接收缓冲区大小失败");
        return -1;
    }

    if (buffer_size < size * 1024) {
        return buffer_size / 1024;
    }

    return 0;
}

int socket_set_send_buf(int fd, uint size)
{
    uint buffer_size = size * 1024;
    int ret = setsockopt(fd, SOL_SOCKET, SO_SNDBUF,
            (const char*) &buffer_size, sizeof(int));
    if (ret == -1) {
        LOG_ERRNO(LOG_LEVEL_WARNING, "设置发送缓冲区大小失败:%d(k)",
                size);
        return -1;
    }

    buffer_size = 0;
    socklen_t buffer_size_len = sizeof(buffer_size);
    ret = getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &buffer_size,
            &buffer_size_len);
    if (ret == -1) {
        LOG_ERRNO(LOG_LEVEL_WARNING, "获取发送缓冲区大小失败");
        return -1;
    }

    if (buffer_size < size * 1024) {
        return buffer_size / 1024;
    }

    return 0;
}
