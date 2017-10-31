/*
 * test_file.c
 *
 *  Created on: 2012-6-26
 *      Author: Administrator
 */

#include <clib/array.h>
#include <clib/file.h>
#include <clib/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void test_file()
{
    str_array_t *array = str_array_new_full(10000);
    int ret = file_readlines("/root/test.logab", array);
    if (ret == -1) {
        LOG(LOG_LEVEL_TEST, "读取文件失败:/root/test.log");
        str_array_free(array);
        return;
    }
//    for (uint i = 0; i < str_array_size(array); i++) {
//        printf("%s\n", str_array_at(array, i));
//    }
    ret = file_writelines("/root/test1.log", array);
    if (ret == -1) {
        LOG(LOG_LEVEL_TEST, "写文件失败:/root/test1.log");
    }
    str_array_free(array);
//    int ret = file_appendline("/root/test.log", "hello world");
//    LOG_RET_IF_ERR(ret, LOG_LEVEL_TEST, "appendline失败:/root/test.log");
}
