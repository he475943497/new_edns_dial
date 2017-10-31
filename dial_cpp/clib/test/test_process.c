#include "clib/array.h"

#include <stdio.h>
#include <stdlib.h>
#include <clib/log.h>
#include <string.h>
#include <clib/memory.h>
#include <clib/string.h>
#include <unistd.h>
#include <clib/log.h>
#include "clib/process_info.h"


void test_process_info()
{
//    struct proc_stat cur_stat;
//    process_stat(getpid(), &cur_stat);
//    ptr_array_t *test_arr = ptr_array_new_full(8, default_destroy_func);
//    process(cur_stat.name, test_arr);
//    ptr_array_free(test_arr);
//    if (process_is_uniq())
//        sleep(10000); //为了多开几个测试process
//    else
//        printf("i am not the first one\n");

    struct proc_stat_ext stat_ext;
    process_stat_ext_by_pid(1060, &stat_ext);
    LOG(LOG_LEVEL_TEST, "%s %d %d %d", stat_ext.name, stat_ext.pid, stat_ext.cpu_use, stat_ext.mem_use);

}
