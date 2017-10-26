/**
 *Author:           wuyangchun
 *Date:             2012-06-01
 *Description:      获取进程相关信息
 *
 *Notice:
 **/

#ifndef CLIB_PROCESS_INFO_H_
#define CLIB_PROCESS_INFO_H_

#include <clib/types.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <clib/array.h>

//结构体中的信息待明确

#ifdef __cplusplus
extern "C" {
#endif

//copy from procs ps end

struct proc_stat {
    char name[MAX_PATH_NAME];       //名称
//    int pri;                        //动态优先级
//    int nice;                       //静态优先级
    uint mem_size;                  //虚拟地址空间大小（k）
    uint rss;                   //当前驻留物理地址空间的大小（k），即我们常说的内存占用
    int state;                      //任务的状态,暂时没有值，R:runnign, S:sleeping (TASK_INTERRUPTIBLE), D:disk sleep
    time_t utime;                    //在用户态运行时间
    time_t stime;                       //在内核态的运行时间
    time_t start_time;              //开始运行时间
    int num_threads;                //线程数
    pid_t pid;
    pid_t ppid;
};

struct proc_stat_ext {
	char name[MAX_PATH_NAME];       //名称
    uint8_t cpu_use;            //cpu占用率
    uint mem_use;                  //占用内存大小（k）
    pid_t pid;                  //进程编号
};

/*
 *@brief        获取指定进程的一些属性信息
 *@param in     pid    进程ID
 *@param out    proc_stat    进程信息
 *@return       -1表示失败
 *@notice       通过读取/proc/pid/stat文件实现
 */
int process_stat(pid_t pid, struct proc_stat* proc_stat);

/*
 *@brief        获取指定进程的一些其他属性信息
 *@param in     name    进程名称
 *@param out    proc_stat_ext    进程信息
 *@return       -1表示失败
 *@notice       通过top/ps命令实现
 */
int process_stat_ext(const char* name, struct proc_stat_ext* proc_stat_ext);

/*
 *@brief        获取指定进程的一些其他属性信息
 *@param in     pid    进程ID
 *@param out    proc_stat_ext    进程信息
 *@return       -1表示失败
 *@notice       通过top/ps命令实现
 */
int process_stat_ext_by_pid(pid_t pid, struct proc_stat_ext* proc_stat_ext);

/*
 *@brief        获取指定进程的可执行文件路径
 *@param in     pid        进程编号
 *@param out    dir        获取到的可执行文件路径
 *@param in     dir_len    存放dir的长度
 *@return       -1表示失败
 *@notice       通过读取/proc/pid/exe文件实现
 */
int process_dir(pid_t pid, char *dir, uint dir_len);



/*
 * 在系统中查找所有进程，将进程名与给定的名称相同的（包括当前进程）放进
 * 给定的array中
 * @param in name: 进程名称
 * @param out stat_arr: 进程信息保存的数组
 * @return 成功，返回true， 否则返回false
 */
extern bool process(const char *name, ptr_array_t* stat_arr);

/*
 *	@brief 检测系统中是否还有与当前进程名称一样的进程，即进程的exename段，
 *	/proc/pid/stat文件中的第二个字段相同
 *
 * @return 如果存在这样的进程（至少1个）返回false
 * 否则返回true
 */
extern bool process_is_uniq(void);



#ifdef __cplusplus
}
#endif

#endif /* CLIB_PROCESS_INFO_H_ */
