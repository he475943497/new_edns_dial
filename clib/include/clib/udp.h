/*
 * udp.h
 *
 *  Created on: 2012-11-8
 *      Author: yamu
 */

#ifndef CLIB_UDP_H_
#define CLIB_UDP_H_

#include <clib/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <clib/ip.h>


#ifdef __cplusplus
extern "C" {
#endif


/*
 *@brief        创建一个udp socket， ，并绑定到指定IP和端口
 *@param in     ip          绑定的服务器IP，必须传入一个值，否则内部不知道是创建IPV6还是IPV4 socket(网络序)
 *@param in     port        绑定的服务器端口，如果为0，则绑定随机端口(主机序)
 *@param in     nonblock    生成的socket是否非阻塞模式
 @return       -1失败，否则成功
 */
int udp_new_socket(const struct ip_addr *ip, uint16_t port,
        bool nonblock);

/*
 *@brief        向指定IP，端口发送数据
 *@param in     fd
 *@param in     data        数据
 *@param in     data_len    数据长度
 *@param in     ip          数据发往的地址
 *@param in     port        数据发往的端口
 @return       -1失败，否则成功
 */
int udp_sendto(int fd, char* data, uint data_len, const struct ip_addr* ip, uint16_t port);

#ifdef __cplusplus
}
#endif

#endif /* UDP_H_ */
