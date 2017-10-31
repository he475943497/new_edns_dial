/*
 * test_file.c
 *
 *  Created on: 2012-6-26
 *      Author: Administrator
 */

#include <clib/file_system.h>
#include <clib/file.h>
#include <clib/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void test_file_system()
{
    LOG(LOG_LEVEL_TEST, "剩余空间大小：%dM", free_space("/"));
}
