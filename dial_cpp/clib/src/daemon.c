/*
 * daemonize.c
 *
 *  Created on: 2012-12-6
 *      Author: ry
 */

#include <clib/daemon.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <limits.h>
#include <clib/array.h>
#include <errno.h>
#include <signal.h>
#include <clib/signal.h>
#include <clib/process_info.h>

static int_array_t *g_pid_arr;

static void term_handler(int signo)
{
    //printf("kill all child and return\n");
    for (int i = 0; i < int_array_size(g_pid_arr); i++) {
        pid_t cur_pid = int_array_at(g_pid_arr, i);
        kill(cur_pid, SIGTERM);
        waitpid(cur_pid, NULL, WNOHANG);
    }
    int_array_free(g_pid_arr);
    exit(EXIT_SUCCESS);
}
static void babysit(int_array_t *pid_arr)
{
    while (1) {
        sleep(1);
        //printf("babysit\n");
        for (int i = 0; i < int_array_size(pid_arr); i++) {
            pid_t baby_pid = (pid_t) int_array_at(pid_arr, i);
            int ret = waitpid(baby_pid, NULL, WNOHANG);
            if (ret > 0) {
                //printf("a process exit\nfork a new one\n");
                pid_t new_baby = fork();
                switch (new_baby) {
                case -1:
                    exit(EXIT_FAILURE);
                case 0:
                    signal_install(SIGTERM, SIG_DFL);
                    return;
                default:
                    int_array_set(g_pid_arr, i, new_baby);
                    break;
                }
            }
        }/*end of for*/
    }/* end of while*/
}

static int do_fork()
{
    pid_t ret = fork();
    if (ret == 0)
        return 0;
    else if (ret > 0) {
        int_array_append(g_pid_arr, ret);
        return ret;
    } else
        /*fork fail*/
        return -1;
}

static bool daemon_is_uniq()
{
    return process_is_uniq();
}

void daemon_start(uint process_count)
{
    if (process_count == 0)
        return;
    struct proc_stat cur_stat;
    memset(&cur_stat, 0, sizeof(cur_stat));
    process_stat(getpid(), &cur_stat);
    if (!daemon_is_uniq()) {
        printf("process %s already exist, start failed\n", cur_stat.name);
        exit(EXIT_FAILURE);
    }
    g_pid_arr = int_array_new_sized(process_count);
    if (!g_pid_arr) {
        //printf("not enough memory,exit\n");
        exit(EXIT_FAILURE);
    }
    printf("start %s success\n", cur_stat.name);
    daemon(1, 0);

    for (int i = 0; i < process_count; i++) {
        int ret = do_fork();
        if (ret == 0)
            return;
    }

    signal_install(SIGTERM, term_handler);
    babysit(g_pid_arr);
}

void daemon_stop()
{
    struct proc_stat cur_stat;
    process_stat(getpid(), &cur_stat);
    ptr_array_t* stat_arr = ptr_array_new_full(4, default_destroy_func);
    process(cur_stat.name, stat_arr);
    if (ptr_array_size(stat_arr) == 1) {
        printf("process %s doesn't exist, stop failed\n", cur_stat.name);
    } else {
        for (uint i = 0; i < ptr_array_size(stat_arr); i++) {
            struct proc_stat* temp_stat = ptr_array_at(stat_arr, i);
            if (temp_stat->pid == getpid()) {
                continue;
            }
            struct proc_stat ppid_stat;
            process_stat(temp_stat->ppid, &ppid_stat);
            if (strcmp(ppid_stat.name, temp_stat->name) != 0) { //父进程和自己不同名
                kill(temp_stat->pid, SIGTERM);
                ptr_array_remove_all(stat_arr);
                while (1) {
                    process(cur_stat.name, stat_arr);
                    if (ptr_array_size(stat_arr) == 1) {
                        break;
                    } else {
                        usleep(1000);
                        ptr_array_remove_all(stat_arr);
                    }
                }
                break;
            }
        }
        printf("stop %s success\n", cur_stat.name);
        //exit(EXIT_SUCCESS);  //comment for restart
    }
    ptr_array_free(stat_arr);
}

