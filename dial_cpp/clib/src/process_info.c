#include <clib/process_info.h>
#include <sys/types.h>
#include <unistd.h>
//#include <asm/page.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <clib/log.h>
#include <clib/string.h>
#include <clib/file_system.h>
#include <clib/memory.h>


/*
 * 下面的一些定义是从procps源码中提取出来，procps主要是提供
 * 诸如ps/pgrep/top/uptime/vmstat等系统命令的一个应用包，这里的
 * 关于系统信息的读取应该有些“权威”性，所以摘抄了部分过来，
 * 下面是proc_t结构体的定义，即/proc/pid/stat文件解析对应的一个结构体。
 * 增加了一个api： process_stat_std用于“填充”此字段
 *
 *
 * 下载此源码Debian/Ubuntu上用apt-get  source procps即可，Redhat/Fedora上用
 * which ps |rpm -qf |yumdowloader --source下载此源码包
 */


#if defined(k64test) || (defined(_ABIN32) && _MIPS_SIM == _ABIN32)
#define KLONG long long    // not typedef; want "unsigned KLONG" to work
#define KLF "L"
#define STRTOUKL strtoull
#else
#define KLONG long
#define KLF "l"
#define STRTOUKL strtoul
#endif

#define P_G_SZ 20

#define SIGNAL_STRING
#define QUICK_THREADS        /* copy (vs. read) some thread info from parent proc_t */



// ld   cutime, cstime, priority, nice, timeout, alarm, rss,
// c    state,
// d    ppid, pgrp, session, tty, tpgid,
// s    signal, blocked, sigignore, sigcatch,
// lu   flags, min_flt, cmin_flt, maj_flt, cmaj_flt, utime, stime,
// lu   rss_rlim, start_code, end_code, start_stack, kstk_esp, kstk_eip,
// lu   start_time, vsize, wchan,

// This is to help document a transition from pid to tgid/tid caused
// by the introduction of thread support. It is used in cases where
// neither tgid nor tid seemed correct. (in other words, FIXME)
#define XXXID tid

// Basic data structure which holds all information we can get about a process.
// (unless otherwise specified, fields are read from /proc/#/stat)
//
// Most of it comes from task_struct in linux/sched.h
//
typedef struct proc_t {
// 1st 16 bytes
    int
        tid,        // (special)       task id, the POSIX thread ID (see also: tgid)
        ppid;       // stat,status     pid of parent process
    unsigned
        pcpu;           // stat (special)  %CPU usage (is not filled in by readproc!!!)
    char
        state,      // stat,status     single-char code for process state (S=sleeping)
#ifdef QUICK_THREADS
        pad_1,          // n/a             padding (psst, also used if multi-threaded)
#else
        pad_1,          // n/a             padding
#endif
        pad_2,      // n/a             padding
        pad_3;      // n/a             padding
// 2nd 16 bytes
    unsigned long long
    utime,      // stat            user-mode CPU time accumulated by process
    stime,      // stat            kernel-mode CPU time accumulated by process
// and so on...
    cutime,     // stat            cumulative utime of process and reaped children
    cstime,     // stat            cumulative stime of process and reaped children
    start_time; // stat            start time of process -- seconds since 1-1-70
#ifdef SIGNAL_STRING
    char
    // Linux 2.1.7x and up have 64 signals. Allow 64, plus '\0' and padding.
    signal[18], // status          mask of pending signals, per-task for readtask() but per-proc for readproc()
    blocked[18],    // status          mask of blocked signals
    sigignore[18],  // status          mask of ignored signals
    sigcatch[18],   // status          mask of caught  signals
    _sigpnd[18];    // status          mask of PER TASK pending signals
#else
    long long
    // Linux 2.1.7x and up have 64 signals.
    signal,     // status          mask of pending signals, per-task for readtask() but per-proc for readproc()
    blocked,    // status          mask of blocked signals
    sigignore,  // status          mask of ignored signals
    sigcatch,   // status          mask of caught  signals
    _sigpnd;    // status          mask of PER TASK pending signals
#endif
    unsigned KLONG
    start_code, // stat            address of beginning of code segment
    end_code,   // stat            address of end of code segment
    start_stack,    // stat            address of the bottom of stack for the process
    kstk_esp,   // stat            kernel stack pointer
    kstk_eip,   // stat            kernel instruction pointer
    wchan;      // stat (special)  address of kernel wait channel proc is sleeping in
    long
    priority,   // stat            kernel scheduling priority
    nice,       // stat            standard unix nice level of process
    rss,        // stat            identical to 'resident'
    alarm,      // stat            ?
    // the next 7 members come from /proc/#/statm
    size,       // statm           total virtual memory (as # pages)
    resident,   // statm           resident non-swapped memory (as # pages)
    share,      // statm           shared (mmap'd) memory (as # pages)
    trs,        // statm           text (exe) resident set (as # pages)
    lrs,        // statm           library resident set (always 0 w/ 2.6)
    drs,        // statm           data+stack resident set (as # pages)
    dt;     // statm           dirty pages (always 0 w/ 2.6)
    unsigned long
    vm_size,        // status          equals 'size' (as kb)
    vm_lock,        // status          locked pages (as kb)
    vm_rss,         // status          equals 'rss' and/or 'resident' (as kb)
    vm_data,        // status          data only size (as kb)
    vm_stack,       // status          stack only size (as kb)
    vm_swap,        // status          based on linux-2.6.34 "swap ents" (as kb)
    vm_exe,         // status          equals 'trs' (as kb)
    vm_lib,         // status          total, not just used, library pages (as kb)
    rtprio,     // stat            real-time priority
    sched,      // stat            scheduling class
    vsize,      // stat            number of pages of virtual memory ...
    rss_rlim,   // stat            resident set size limit?
    flags,      // stat            kernel flags for the process
    min_flt,    // stat            number of minor page faults since process start
    maj_flt,    // stat            number of major page faults since process start
    cmin_flt,   // stat            cumulative min_flt of process and child processes
    cmaj_flt;   // stat            cumulative maj_flt of process and child processes
    char
        **environ,      // (special)       environment string vector (/proc/#/environ)
        **cmdline,      // (special)       command line string vector (/proc/#/cmdline)
        **cgroup,       // (special)       cgroup string vector (/proc/#/cgroup)
         *supgid,       // status          supplementary gids as comma delimited str
         *supgrp;       // supp grp names as comma delimited str, derived from supgid
    char
    // Be compatible: Digital allows 16 and NT allows 14 ???
        euser[P_G_SZ],  // stat(),status   effective user name
        ruser[P_G_SZ],  // status          real user name
        suser[P_G_SZ],  // status          saved user name
        fuser[P_G_SZ],  // status          filesystem user name
        rgroup[P_G_SZ], // status          real group name
        egroup[P_G_SZ], // status          effective group name
        sgroup[P_G_SZ], // status          saved group name
        fgroup[P_G_SZ], // status          filesystem group name
        cmd[NAME_MAX];  // stat,status     basename of executable file in call to exec(2)
    struct proc_t
    *ring,      // n/a             thread group ring
    *next;      // n/a             various library uses
    int
    pgrp,       // stat            process group id
    session,    // stat            session id
    nlwp,       // stat,status     number of threads, or 0 if no clue
    tgid,       // (special)       thread group ID, the POSIX PID (see also: tid)
    tty,        // stat            full device number of controlling terminal
    /* FIXME: int uids & gids should be uid_t or gid_t from pwd.h */
        euid, egid,     // stat(),status   effective
        ruid, rgid,     // status          real
        suid, sgid,     // status          saved
        fuid, fgid,     // status          fs (used for file access only)
    tpgid,      // stat            terminal process group id
    exit_signal,    // stat            might not be SIGCHLD
    processor;      // stat            current (or most recent?) CPU
#ifdef OOMEM_ENABLE
    int
        oom_score,      // oom_score       (badness for OOM killer)
        oom_adj;        // oom_adj         (adjustment to OOM score)
#endif
} proc_t;


int process_stat_ext(const char* name, struct proc_stat_ext* proc_stat_ext)
{
    char *filename = "clib_pro_file";
    char buf[1024] = { 0 };
    char cmd[1024] = { 0 };

    if (strncmp(name, "topn", strlen(name)) == 0) {
        system(
                "ps -eo comm,pcpu,rss,pid|grep 'topn '|grep -v 'start_topn' > clib_pro_file");
    } else {
        snprintf(cmd, sizeof(cmd),
                "ps -eo comm,pcpu,rss,pid|grep '%s ' > clib_pro_file", name);
        system(cmd);
    }
    float cpu_use = 0;
    uint32_t mem_use = 0;
    uint32_t pid = 0;
    FILE *fp = fopen(filename, "r");
    LOG_ERRNO_RET_ERR_IF_NULL(fp, LOG_LEVEL_WARNING, "打开文件失败:%s", filename);
    while (fgets(buf, sizeof(buf), fp)) {
        if (strncmp(buf, name, strlen(name)) != 0) {
            continue;
        }
        //printf("buf : %s\n", buf);
        sscanf(buf, "%*s%*[ ]%f%*[ ]%u%*[ ]%u", &cpu_use, &mem_use, &pid);
        bzero(buf, sizeof(buf));

        proc_stat_ext->pid = pid;
        proc_stat_ext->cpu_use = cpu_use;
        proc_stat_ext->mem_use = mem_use;
        //printf("process name:%s\n", name);
        //printf("process cpu_use:%f\n", cpu_use);
        //printf("process mem_use:%d\n", mem_use);
        strcpy(proc_stat_ext->name, name);

        fclose(fp);
        system("rm -f clib_pro_file");
        return 0;
    }
    fclose(fp);
    LOG(LOG_LEVEL_WARNING, "进程未启动: %s", name);
    system("rm -f clib_pro_file");

    return -1;
}

int process_stat_ext_by_pid(pid_t pid, struct proc_stat_ext* proc_stat_ext)
{
    char *filename = "clib_pro_file";
    char buf[1024] = { 0 };
    char cmd[1024] = { 0 };
    char pid_str[16] = {0};
    char *pbuf = NULL;

    sprintf(pid_str, "%d", pid);

	snprintf(cmd, sizeof(cmd),
			"ps -eo pid,pcpu,rss,comm|grep '%s ' > clib_pro_file", pid_str);
	system(cmd);

    float cpu_use = 0;
    uint32_t mem_use = 0;
    char process_name[MAX_PATH_NAME];       //名称
    FILE *fp = fopen(filename, "r");
    LOG_ERRNO_RET_ERR_IF_NULL(fp, LOG_LEVEL_WARNING, "打开文件失败:%s", filename);
    while (fgets(buf, sizeof(buf), fp)) {
    	pbuf = strtrim(buf, strlen(buf));
        if (strncmp(pbuf, pid_str, strlen(pid_str)) != 0) {
            continue;
        }
        sscanf(pbuf, "%*s%*[ ]%f%*[ ]%u%*[ ]%s", &cpu_use, &mem_use, process_name);
        bzero(buf, sizeof(buf));

        proc_stat_ext->pid = pid;
        proc_stat_ext->cpu_use = cpu_use;
        proc_stat_ext->mem_use = mem_use;
        strcpy(proc_stat_ext->name, process_name);

        fclose(fp);
        system("rm -f clib_pro_file");
        return 0;
    }
    fclose(fp);
    LOG(LOG_LEVEL_WARNING, "进程未启动: %s", pid_str);
    system("rm -f clib_pro_file");

    return -1;
}




// Reads /proc/*/stat files, being careful not to trip over processes with
// names like ":-) 1 2 3 4 5 6".
static void __stat2proc(const char* S, proc_t *restrict P) {
    unsigned num;
    char* tmp;



    /* fill in default values for older kernels */
    P->processor = 0;
    P->rtprio = -1;
    P->sched = -1;
    P->nlwp = 0;

    S = strchr(S, '(') + 1;
    tmp = strrchr(S, ')');
    num = tmp - S;
    if(num >= sizeof P->cmd) num = sizeof P->cmd - 1;
    memcpy(P->cmd, S, num);
    P->cmd[num] = '\0';
    S = tmp + 2;                 // skip ") "

    num = sscanf(S,
       "%c "
       "%d %d %d %d %d "
       "%lu %lu %lu %lu %lu "
       "%Lu %Lu %Lu %Lu "  /* utime stime cutime cstime */
       "%ld %ld "
       "%d "
       "%ld "
       "%Lu "  /* start_time */
       "%lu "
       "%ld "
       "%lu %"KLF"u %"KLF"u %"KLF"u %"KLF"u %"KLF"u "
       "%*s %*s %*s %*s " /* discard, no RT signals & Linux 2.1 used hex */
       "%"KLF"u %*lu %*lu "
       "%d %d "
       "%lu %lu",
       &P->state,
       &P->ppid, &P->pgrp, &P->session, &P->tty, &P->tpgid,
       &P->flags, &P->min_flt, &P->cmin_flt, &P->maj_flt, &P->cmaj_flt,
       &P->utime, &P->stime, &P->cutime, &P->cstime,
       &P->priority, &P->nice,
       &P->nlwp,
       &P->alarm,
       &P->start_time,
       &P->vsize,
       &P->rss,
       &P->rss_rlim, &P->start_code, &P->end_code, &P->start_stack, &P->kstk_esp, &P->kstk_eip,
/*     P->signal, P->blocked, P->sigignore, P->sigcatch,   */ /* can't use */
       &P->wchan, /* &P->nswap, &P->cnswap, */  /* nswap and cnswap dead for 2.4.xx and up */
/* -- Linux 2.0.35 ends here -- */
       &P->exit_signal, &P->processor,  /* 2.2.1 ends with "exit_signal" */
/* -- Linux 2.2.8 to 2.5.17 end here -- */
       &P->rtprio, &P->sched  /* both added to 2.5.18 */
    );

    if(!P->nlwp){
      P->nlwp = 1;
    }
}


static int process_stat_std(pid_t pid, proc_t *proc_info)
{
	char stat_file[PATH_MAX] = {};
	snprintf(stat_file, PATH_MAX, "/proc/%d/stat", pid);
	int fd = open(stat_file, O_RDONLY, 0);
	if (fd == -1)
		return -1;

	char buf[1024] = {};
	int num_read = 0;
	num_read = read(fd, buf, 1023);
	close(fd);
	if (num_read < 0)
		return -1;
	buf[num_read] = '\0';
	__stat2proc(buf, proc_info);

	return 0;
}

int process_stat(pid_t pid, struct proc_stat *proc_stat)
{
	if (!proc_stat)
		return -1;


	struct proc_t full_proc_info;
	int full_ret = process_stat_std(pid, &full_proc_info);
	if (full_ret == -1)
		return -1;

	strcpy(proc_stat->name, full_proc_info.cmd);
	proc_stat->mem_size = full_proc_info.vsize / 1024;
	proc_stat->rss = full_proc_info.rss;
	proc_stat->state = full_proc_info.state;
	proc_stat->utime = full_proc_info.utime;
	proc_stat->stime = full_proc_info.stime;
	proc_stat->start_time = full_proc_info.start_time;
	proc_stat->num_threads = full_proc_info.nlwp;
    proc_stat->pid = pid;
    proc_stat->ppid = full_proc_info.ppid;

	return 0;
}



static bool proc_filt_pid(const char *dir, const char *subdir)
{
	if (*subdir > '0' && *subdir <= '9')
		return true;
	return false;
}





bool process(const char *name, ptr_array_t* stat_arr)
{
    if (!name || !stat_arr)
        return false;
    str_array_t *os_pid_arr = str_array_new_full(128);
	int ret = subdirs("/proc", os_pid_arr, proc_filt_pid);
	if (ret == -1) {
		str_array_free(os_pid_arr);
		return false;
	}
    struct proc_stat *cur_stat = NULL;
    for (int i = 0; i < str_array_size(os_pid_arr); i++) {
        const char *cur_pid_str = str_array_at(os_pid_arr, i);
        pid_t cur_pid = atoi(cur_pid_str);
        cur_stat = mem_alloc(sizeof(*cur_stat));
        if (process_stat(cur_pid, cur_stat) == 0) {
            if (strcmp(name, cur_stat->name) == 0) {
                //printf("just padding for gdb\n");
                ptr_array_append(stat_arr, cur_stat);
            } else 
                mem_free(cur_stat);
        }else{
            str_array_free(os_pid_arr);
            return false;
        }
    }
    str_array_free(os_pid_arr);
    return true;
}

bool process_is_uniq()
{
    pid_t cur_pid = getpid();
    struct proc_stat cur_stat;
    memset(&cur_stat, 0, sizeof(cur_stat));
    process_stat(cur_pid, &cur_stat);
    ptr_array_t *stat_arr = ptr_array_new_full(8, default_destroy_func);
    process(cur_stat.name, stat_arr);
    if (ptr_array_size(stat_arr) == 1) { //only contain it self
        ptr_array_free(stat_arr);
        return true;
    } else {
        ptr_array_free(stat_arr);
        return false;
    }
}

int process_dir(pid_t pid, char *dir, uint dir_len)
{
    static char buf_tmp[MAX_PATH_NAME];
    snprintf(buf_tmp, sizeof(buf_tmp), "%s%d%s", "/proc/", pid, "/exe");
    int rslt = readlink(buf_tmp, dir, dir_len);
    if (rslt < 0 || rslt >= dir_len) {
        return -1;
    }
    char *last = strlast(dir, '/');
    if (last == NULL) {
        LOG(LOG_LEVEL_ERROR, "get strlast is failed");
        return -1;
    }
    *last = '\0';
    return 0;
}

