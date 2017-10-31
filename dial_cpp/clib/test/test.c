/*
 * test.c
 *
 *  Created on: 2012-6-1
 *      Author: Administrator
 */

#include <stdio.h>
#include "test.h"
#include <clib/log.h>
#include <clib/memory.h>

int main(int argc, char **argv)
{
    test_string();
    //    test_log();
    //    test_file();

    //test_hash_table();
    //    test_list();
    //    test_tree();
    //    test_topn();
    //    test_seg_tree();
    //test_array();
    //    test_socket();
    //    test_timer();
    //test_ini();
    //    test_epoll();
    //    test_queue();

    //    test_file_system();
    //test_daemon(argc, argv);
    //    test_process_info();
    /* test_system_stat(); */
    //    test_iface();
    /* test_time(); */



#ifdef CHECK_MEM_LEAK
    mem_leak_dump();
#endif

    return 0;
}
