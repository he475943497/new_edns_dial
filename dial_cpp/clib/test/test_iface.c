/*
 * test_iface.c
 *
 *  Created on: 2012-12-13
 *      Author: ry
 */


#include <clib/array.h>
#include <clib/iface.h>
#include <stdio.h>

void test_iface()
{
	ptr_array_t *addr_arr = ptr_array_new_full(6, default_destroy_func);
	iface_addr(addr_arr);
	for (int i = 0; i < ptr_array_size(addr_arr); i++) {
		char test_ip_str[INET6_ADDRSTRLEN] = {};
		ip2str(ptr_array_at(addr_arr, i), test_ip_str, INET6_ADDRSTRLEN);
		printf("%s\n", test_ip_str);
	}
	ptr_array_free(addr_arr);
}
