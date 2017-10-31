/*
 * test_system_stat.c
 *
 *  Created on: 2013-5-3
 *      Author: yamu
 */

#include <clib/system_stat.h>
#include <clib/log.h>

void test_system_stat()
{
	struct mem_stat stat;
	memory_stat(&stat);
	LOG(LOG_LEVEL_TEST, "Total:%u, Free:%u", stat.total, stat.free);
}
