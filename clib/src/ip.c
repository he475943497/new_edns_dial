/*
 * ip.c
 *
 *  Created on: 2012-7-31
 *      Author: Administrator
 */
#include <clib/ip.h>

uint ipv4_to_str(uint32_t ipv4, char *dst, uint len)
{
    if (len < INET_ADDRSTRLEN) {
        return 0;
    }

    char *in = (char *) &ipv4;
    uint index = 0;
    for (int i = 0; i < 4; i++) {
        uint8_t data = in[i];
        if (data >= 100) {
            dst[index++] = '0' + data / 100;
            dst[index++] = '0' + (data % 100) / 10;
            dst[index++] = '0' + data % 10;
        } else if (data >= 10) {
            dst[index++] = '0' + data / 10;
            dst[index++] = '0' + data % 10;
        } else {
            dst[index++] = '0' + data;
        }

        if (i != 3) {
            dst[index++] = '.';
        }
    }
    dst[index] = '\0';

    return index;
}

int ip2str(struct ip_addr *in_addr, char *ip_str, size_t len)
{
    if (!in_addr || !ip_str)
        return -1;
    char temp[60] = { };
    if (in_addr->version == IPV4) {
        if (inet_ntop(AF_INET, &in_addr->addr.ipv4, temp, 16) != NULL) {
            if (len < strlen(temp) + 1)
                return -1;
            strcpy(ip_str, temp);
        } else
            return -1;
    } else if (in_addr->version == IPV6) {
        if (inet_ntop(AF_INET6, &in_addr->addr.ipv6, temp, 50) != NULL) {
            if (len < strlen(temp) + 1)
                return -1;
            strcpy(ip_str, temp);
        }
    } else
        return -1;
    return 0;
}

int str2ip(const char *str, struct ip_addr *addr)
{
    char address[16];
    if (inet_pton(AF_INET, str, address) > 0) {
        addr->version = IPV4;
        memcpy(&addr->addr.ipv4, address, 4);
        return 0;
    } else if (inet_pton(AF_INET6, str, address) > 0) {
        addr->version = IPV6;
        memcpy(&addr->addr.ipv6, address, 16);
        return 0;
    }
    return -1;
}
