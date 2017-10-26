/**
 *Author:           wuyangchun
 *Date:             2012-07-26
 *Description:      IP地址相关
 *
 *Notice:
 **/


#ifndef CLIB_IP_H_
#define CLIB_IP_H_

#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IPV4 4
#define IPV6 6

//#define MAX_IPV4_STR_LEN 16                   //最大IPV4串长度
//#define MAX_IPV6_STR_LEN 40                   //最大IPV6串长度
#define MAX_IPV4_STR_LEN INET_ADDRSTRLEN                  //最大IPV4串长度
#define MAX_IPV6_STR_LEN INET6_ADDRSTRLEN                  //最大IPV6串长度

#pragma pack(push)
#pragma pack(1)

struct ip_addr {
    int8_t version;    //IPV4 or IPV6
    union {
        uint32_t ipv4;
        struct in6_addr ipv6;
    } addr;
};

//为了确保高速，不检查IPV4是否是合法地址,返回地址长度
uint ipv4_to_str(uint32_t ipv4, char *dst, uint len);
/*
 * 将上面定义的结构体中的ip地址部分转化成str输出
 */
int ip2str(struct ip_addr *in_addr, char *str, size_t len);

/*
 * 将上面定义的结构体中的ip地址部分转化成str输出
 */
int str2ip(const char *str, struct ip_addr *in_addr);

#pragma pack(pop)


#ifdef __cplusplus
}
#endif

#endif /* CLIB_IP_H_ */
