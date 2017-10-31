/**
 *Author:           wuyangchun
 *Date:             2012-06-01
 *Description:      获取系统相关信息
 *
 *Notice:
 **/

#ifndef CLIB_SYSINFO_H
#define CLIB_SYSINFO_H

#include <clib/types.h>
#include <clib/array.h>


#ifdef __cplusplus
extern "C" {
#endif

//下面两个结构体里面的数量单位待定
struct cpu_stat {
    uint16_t core_number;          //CPU内核数目
//    uint frequency;                     //CPU频率
//    uint utilization;                   //CPU利用率
    uint64_t total;
    uint64_t idle;

};

struct mem_stat {
    uint total;             //总内存数(K)
    uint free;              //空闲内存数(K)
    uint cached;            //cached(K)
};

#define MAX_PATITION_NAME 256           //最大分区名称长度
struct partition_stat {
    uint32_t size;                  //分区大小（M）
    uint32_t used;                  //已经占用（M）
    char name[MAX_PATITION_NAME];   //分区名称

} disk_t;

struct disk_info {
    uint partition_number;              //分区数
    uint free;                          //空闲内存数
    uint cached;
};

#define MAX_NIC_NAME 128
struct nic_stat {
    uint64_t in_count;       //接收的数据量(byte)
    uint64_t out_count;      //发送的数据量(byte)
    char name[MAX_NIC_NAME];   //网卡名称
};


/*
 *@brief        获取系统CPU信息(读取/proc/cpuinfo)
 *@param out    cpu_stat    CPU信息
 *@return       -1表示失败
 */
int cpu_stat(struct cpu_stat* cpu_stat);

/*
 *@brief        获取系统内存信息(读取/proc/meminfo)
 *@param out    mem_stat    内存信息
 *@return       -1表示失败
 */
int memory_stat(struct mem_stat* mem_stat);

/*
 *@brief        获取系统分区信息(利用df命令)
 *@param out    partition_stat    内存信息
 *@return       -1表示失败
 */
int partition_stat(array_t* partition_stat);

int nic_stat(array_t* nic_stat);


#ifdef __cplusplus
}
#endif

#endif  /* CLIB_SYSINFO_H */
