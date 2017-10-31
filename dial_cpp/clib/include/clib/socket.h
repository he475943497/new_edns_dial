/**
 *Author:           wuyangchun
 *Date:             2012-07-25
 *Description:      socket相关操作
 *
 *Notice:           封装起来只是为了便于调用，不保证高效率
 **/

#ifndef CLIB_SOCKET_H_
#define CLIB_SOCKET_H_

#include <clib/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <clib/ip.h>
#include <clib/udp.h>
#include <clib/tcp.h>

#ifdef __cplusplus
extern "C" {
#endif


#if __BYTE_ORDER == __LITTLE_ENDIAN
    #define ntohll(n)  ((uint64_t)ntohl((uint64_t)(n) >> 32) |\
                        ((uint64_t)ntohl(((uint64_t)(n) << 32)>>32))<<32)
    #define htonll(n) ntohll(n)
#else
    #define ntohll(n) n
    #define htonll(n) n
#endif


/*
 *@brief        创建一个raw socket
 *@param in     eth         绑定网卡名称，如果传入NULL或者空字符串，则不和具体网卡绑定
 *@param in     recv_buf    接收缓存大小（单位K）
 *@param in     nonblock    是否非阻塞模式
 *@return       -1失败，否则成功
 */
int socket_new_raw_server(const char *eth, uint recv_buf, bool nonblock);


/*
 *@brief        设置接收缓冲区大小
 *@param in     fd          socket描述符
 *@param in     size        将要设置的缓冲区大小（K）
 *@return       -1表示发生异常，0表示设置成功，否则返回当前实际的buffer大小（未发生异常，但也未设置成功的情况）
 *
 */
int socket_set_recv_buf(int fd, uint size);
int socket_set_send_buf(int fd, uint size);

/*
 *@brief        设置socket阻塞模式
 *@param in     fd          socket描述符
 *@param in     blocking    是否阻塞
 *@return       true/false  成功/失败
 *
 */
bool socket_set_blocking(int sockfd, bool blocking);

#ifdef __cplusplus
}
#endif

#endif /* SOCKET_H_ */
