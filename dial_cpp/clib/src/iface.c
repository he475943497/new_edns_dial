/*
 * iface.c
 *
 *  Created on: 2012-12-13
 *      Author: ry
 */

#include <clib/iface.h>
#include <clib/ip.h>
#include <clib/memory.h>
#include <string.h>

int iface_addr(ptr_array_t *addr_arr)
{
	if (!addr_arr)
		return -1;
	struct ifaddrs *all_addr;
	int ret = getifaddrs(&all_addr);
	if (ret == -1)
		return -1;
	struct ifaddrs *cur;
	for (cur = all_addr; cur; cur = cur->ifa_next) {
		if (cur->ifa_addr == NULL)
			continue;
		if (cur->ifa_addr->sa_family != AF_INET &&
				cur->ifa_addr->sa_family != AF_INET6)
			continue;

		if (cur->ifa_addr->sa_family == AF_INET) { //ipv4 addr
			struct ip_addr *new_ip = mem_alloc(sizeof(struct ip_addr));
			new_ip->version = IPV4;
			struct sockaddr_in *ipv4_addr = (struct sockaddr_in *)cur->ifa_addr;
			memcpy(&new_ip->addr.ipv4, &ipv4_addr->sin_addr.s_addr, sizeof(uint32_t));

			//test part
//			char test_ip[INET_ADDRSTRLEN] = {};
//			ip2str(new_ip, test_ip, INET_ADDRSTRLEN);
//			printf("%s\n", test_ip);
			//test end

			ptr_array_append(addr_arr, new_ip);
		} else { //ipv6 addr
			struct ip_addr *new_ip = mem_alloc(sizeof(struct ip_addr));
			new_ip->version = IPV6;
			struct sockaddr_in6 *ipv6_addr = (struct sockaddr_in6 *)cur->ifa_addr;
			memcpy(&new_ip->addr.ipv6, &ipv6_addr->sin6_addr, sizeof(struct in6_addr));

			char ip_str[INET6_ADDRSTRLEN] = {};
			ip2str(new_ip, ip_str, INET6_ADDRSTRLEN);
            if (strncmp(ip_str, "fe80", strlen("fe80")) == 0) { //can not bind to this address, drop it
                mem_free(new_ip);
                continue;
            }
			//test part
//			char test_ip[INET6_ADDRSTRLEN] = {};
//			ip2str(new_ip, test_ip,INET6_ADDRSTRLEN);
//			printf("%s\n", test_ip);
			//test end
			ptr_array_append(addr_arr, new_ip);
		}
	}/*end of for*/
	freeifaddrs(all_addr);
	return 0;
}
