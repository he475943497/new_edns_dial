/*
 * tcp.h
 *
 *  Created on: 2012-11-27
 *      Author: yamu
 */

#ifndef CLIB_TCP_H_
#define CLIB_TCP_H_

#include <clib/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <clib/ip.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *@brief        创建一个tcp socket，并且连接到指定的主机
 *@param in     local_ip            本地需要绑定的IP(网络序)，如果不需要绑定指定IP，传入NULL即可
 *@param in     local_port          本地需要绑定的端口(主机序)，如果不需要绑定指定端口，传入0即可
 *@param in     server_ip           服务器IP(网络序)
 *@param in     server_port         服务器端口(主机序)
 *@param in     timeout     连接服务器超时时间，等于0表示马上返回，等于-1表示一直等待
 *@param in     nonblock    是否非阻塞模式
 *@return       -1失败，否则成功
 */
int tcp_new_client(const struct ip_addr *local_ip, uint16_t local_port,
        const struct ip_addr *server_ip, uint16_t server_port, time_t timeout,
        bool nonblock);

/*
 *@brief        创建一个tcp socket，并绑定到指定IP和端口
 *@param in     ip          绑定的服务器IP(网络序)，不能为NULL，因为需要根据ip.version字段来决定绑定IPV4还是IPV6
 *                          如果不需要绑定指定IP，ip.addr字段等于值为0即可
 *@param in     port        绑定的服务器端口(主机序)
 *@param in     nonblock    是否非阻塞模式
 *@return       -1失败，否则成功
 */
int tcp_new_server(const struct ip_addr *ip, uint16_t port,
        bool nonblock);

/*
 *@brief        发送tcp数据
 *@param in     fd          socket描述符
 *@param in     data        需要发送的数据
 *@param in     len         发送数据的长度
 *@param in     timeout     超时值
 *@return       -1表示失败、超时或者被中断，否则成功
 *
 @notice        传入的FD必须是阻塞的
 */
int tcp_send(int fd, const char* data, uint len, int timeout);

/*
 *@brief        接收tcp数据
 *@param in     fd          socket描述符
 *@param in     recv_buf    存放接收数据的buffer
 *@param in     buf_len     需要接收数据的长度
 *@param in     timeout     超时值
 *@return       -1表示失败、超时或者被中断，否则成功
 *
 @notice        传入的FD必须是阻塞的
 */
int tcp_recv(int fd, char* recv_buf, uint buf_len, int timeout);

#ifdef __cplusplus
}
#endif

#endif /* CLIB_TCP_H_ */
