#ifndef CLIB_IFACE_H_
#define CLIB_IFACE_H_
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <netinet/in.h>
#include <clib/array.h>


#ifdef __cplusplus
extern "C" {
#endif

/*
 * @brief: 获取本地所有网卡绑定的ip地址，包括ipv4和ipv6的地址，
 * 封装成ip.h中定义的ip_addr结构体，然后放入传入的数组里
 * @param out: addr_arr,存放结果的数组
 * @return 成功返回0, 失败返回-1
 */
int iface_addr(ptr_array_t *addr_arr);

#ifdef __cplusplus
}
#endif

#endif /* IFACE_H_ */

