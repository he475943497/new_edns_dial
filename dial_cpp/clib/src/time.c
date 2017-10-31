/*
 * time.c
 *
 *  Created on: 2012-6-4
 *      Author: Administrator
 */
#include <time.h>
#include <clib/types.h>
#include <stdio.h>

void time_format(time_t time, char *buf, uint len)
{
    struct tm tm_now;
    localtime_r(&time, &tm_now);

    snprintf(buf, len, "%04d-%02d-%02d %02d:%02d:%02d", tm_now.tm_year + 1900,
            tm_now.tm_mon + 1, tm_now.tm_mday, tm_now.tm_hour, tm_now.tm_min,
            tm_now.tm_sec);
}
