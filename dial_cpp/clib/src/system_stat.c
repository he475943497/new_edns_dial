#include <clib/system_stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <clib/log.h>
#include <string.h>
#include <stdio.h>
#include <mntent.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/vfs.h>
//#define BUFFER_SIZE 32
//#define PROCESSOR_STR "processor"

//static uint16_t get_processor_count();

int cpu_stat(struct cpu_stat* cpu_stat)
{
    if (!cpu_stat)
        return -1;
	cpu_stat->core_number = sysconf(_SC_NPROCESSORS_ONLN);
#define BUFF_SIZE (64 * 1024)
	char buff[BUFF_SIZE] = {};
	int fd = open("/proc/stat", O_RDONLY, 0);
	if (fd == -1)
		return -1;
	read(fd, buff, BUFF_SIZE - 1);

	const char *b = strstr(buff, "cpu ");
	if (b) {
		uint64_t user, nice, sys, idle, iowait, irq, softirq, xxx;
		sscanf(b, 
                "cpu  %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu", 
                &user, &nice, &sys, &idle, &iowait, &irq, &softirq, &xxx);
		cpu_stat->idle = idle;
		cpu_stat->total = user + nice + sys + idle + iowait + irq + softirq + xxx;
	} else {
		close(fd);
		return -1;
	}
	close(fd);
    return 0;
}

//static uint16_t get_processor_count()
//{
//    int read_count;
//    uint16_t processor_count;
//    char buffer[BUFFER_SIZE];
//    char tempbuffer[BUFFER_SIZE];
//    int fd = open("/proc/cpuinfo", O_RDONLY);
//    char *find;
//    int buf_len;
//
//    LOG_ERRNO_RET_ZERO_IF_ERR(fd, LOG_LEVEL_WARNING, "打开失败: /proc/cpuinfo");
//
//    processor_count = 0;
//    buffer[0] = '\0';
//    while (true) {
//        buf_len = strlen(buffer);
//        read_count = read(fd, buffer + buf_len, sizeof(buffer) - 1 - buf_len);
//        if (read_count <= 0) {
//            break;
//        }
//        buffer[read_count + buf_len] = '\0';
//        find = buffer;
//        while ((find = strstr(find, PROCESSOR_STR)) != NULL) {
//            find = find + strlen(PROCESSOR_STR);
//            processor_count++;
//        }
//        strcpy(tempbuffer,
//                buffer + (strlen(buffer) - (strlen(PROCESSOR_STR) - 1)));
//        strcpy(buffer, tempbuffer);
//    }
//
//    LOG_ERRNO_IF_ERR(read_count, LOG_LEVEL_WARNING, "读取文件失败");
//
//    return processor_count;
//}

static uint32_t memory_get(const char* buffer, const char* param) {
	const char *total = strstr(buffer, param);
	if (total) {
		uint32_t result = 0;
		sscanf(total + strlen(param), " %u", &result);
		return result;
	} else {
		return 0;
	}
}

int memory_stat(struct mem_stat* mem_stat)
{
	if (!mem_stat)
		return -1;
	int fd = open("/proc/meminfo", O_RDONLY, 0);
	if (fd == -1)
		return -1;
	char buff[2048] = {};
	read(fd, buff, 2048 - 1);
	//total process
	mem_stat->total = memory_get(buff, "MemTotal:");
	mem_stat->free = memory_get(buff, "MemFree:");
	mem_stat->cached = memory_get(buff, "Cached:");
	uint32_t buffers = memory_get(buff, "Buffers:");

	mem_stat->free = mem_stat->free + mem_stat->cached + buffers;

	close(fd);
	return 0;
}

int partition_stat(array_t* partion_stat_arr)
{
	if (!partion_stat_arr)
		return -1;

	/* the code below are stealed from df source code,
     * you can download with your distro command which is 
	 *  In Debian/Ubuntu:
	 *      apt-get source coreutils
	 *  or 
     *  In Redhat/Fedora:
	 *  	yumdownloader --source coreutils
	 *
	 */

	struct mntent *mnt = NULL;
	FILE *fp = setmntent(MOUNTED, "r");
	if (!fp)
		return -1;
	struct partition_stat par_stat;
	memset(&par_stat, 0, sizeof(par_stat));
	while ((mnt = getmntent(fp))) {
		struct statfs fsd;
		if (statfs(mnt->mnt_dir, &fsd) < 0)
			goto cleanup;
		//ignore vritual partition
		if (fsd.f_blocks == 0)
			continue;

		strcpy(par_stat.name, mnt->mnt_dir);
		/*
		 * partition size: fsd.f_blocks
		 * user available: fsd.f_bavail
		 * root available: fsd.f_bfree
		 * block size: fsd.bsize
		 * used: == size - root_available == fsd.f_locks - fsd.f_bfree
		 */

		uint unit = fsd.f_bsize / 1024;
		/* in case overflow */
		/* the same with df -lP output: size field */
		/* par_stat.size = fsd.f_blocks * unit;  */
		/* the same with df -lP used field */
		/* par_stat.used = (fsd.f_blocks - fsd.f_bfree) * unit;  */

		par_stat.size = (fsd.f_blocks * unit) >> 10; // human readable Mb
		par_stat.used = ((fsd.f_blocks - fsd.f_bfree) * unit) >> 10;
		array_append_val(partion_stat_arr, &par_stat);
		memset(&par_stat, 0, sizeof(par_stat));
	}
	if (endmntent(fp) == 0)
		return -1;

	return 0;
cleanup:
	endmntent(fp);
	return -1;
}

void skip_front_space(char *s, int len){
    int i;
    char *p = s;
    for(i = 0; i < len; i++, p++){
        if(*p != ' ')
            break;
    }
    if(i == len || i == 0)
        return;
    i = len - i;
    while(i > 0){
        *s++ = *p++;
        i--;
    }
}

#define NIC_FILE "/proc/net/dev"
int nic_stat(array_t* nic_stat_ary)
{
    bool find_face = false;
    char nic_name[MAX_NIC_NAME] = { 0 };
    char buf[1024] = { 0 };
    char *key = "face";

    FILE *fp = fopen(NIC_FILE, "r");
    if (!fp) {
        return -1;
    }

    while (fgets(buf, sizeof(buf), fp)) {
        bzero(nic_name, sizeof(nic_name));
        sscanf(buf, "%[^:]", nic_name);
        skip_front_space(nic_name, MAX_NIC_NAME);
        //        printf("nic_name:%s\n", nic_name);
//        LOG(LOG_LEVEL_TRACE, "nic_name:%s\n", nic_name);
        if ((find_face == false) && strncmp(nic_name, key, strlen(key)) == 0) {
            find_face = true;
            continue;
        }

        else if (find_face == true) {
//            nic_name[strlen(nic_name) - 1] = '\0';
            struct nic_stat stat;
            sscanf(buf,
                    "%*[^:]%*[^0-9]%llu%*[ ]%*[0-9]%*[ ]%*[0-9]%*[ ]%*[0-9]%*[ ]%*[0-9]%*[ ]%*[0-9]%*[ ]%*[0-9]%*[ ]%*[0-9]%*[ ]%llu",
                    &stat.in_count, &stat.out_count);
//            LOG(LOG_LEVEL_TRACE, "%s:\t in(%u),out(%u)",
//                    nic_name, stat.in_count, stat.out_count);
//            printf("nic name:%s, in:%u, out:%u\n", nic_name, stat.in_count, stat.out_count);
            strncpy(stat.name, nic_name, MAX_NIC_NAME);
            array_append_val(nic_stat_ary, &stat);

        }
        bzero(buf, sizeof(buf));
    }
    fclose(fp);

    return 0;
}

