#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include "Dial_log.h"
#include "Dial_server.h"




int
log_printf(int level,int type,const char *logfile,const char * fmt,...)
{
	extern dial_cfg_t g_cfg;
	extern bool s_debug_switch;
	extern pthread_mutex_t g_log_lock;
	unsigned char 	buf[MAX_DEBUG_BUFFER_LEN];
    	int 			size;
	va_list 		args;
	unsigned char	filename[128] = {0};

	if(LOG_RUN == type) {
		if(!s_debug_switch || g_cfg.log_level < level)
			return 0;
	}

	memset(buf, 0, MAX_DEBUG_BUFFER_LEN);

	va_start(args, fmt);
	size = vsnprintf((char *)buf, MAX_DEBUG_BUFFER_LEN-1, (char *)fmt, args);
	va_end (args);
	
	//printf("%s", buf);

	if(size > 0) {
		sprintf((char *)filename, "%s%s", g_cfg.log_path,
			logfile);
		pthread_mutex_lock(&g_log_lock);
		// 写入相应日志文件
		write_debug_file(filename, buf);
		pthread_mutex_unlock(&g_log_lock);
	}

	return 0;
}


int
write_debug_file(unsigned char			*filename,
					unsigned char			*buffer)
{
	FILE			*fp = NULL;
	time_t 			timep;
	struct tm 		st_tm;
	int 	ui_size = 0;
	unsigned char 	log_time[30] = { 0 };
	unsigned char 	file_path[50] = { 0 };

	if(!filename || !buffer || strlen((char *)buffer) >= MAX_DEBUG_BUFFER_LEN){

		printf("write_debug_file:invalid parameter!\n");
		return -1;
	}
	
	fp = fopen((char *)filename, "a+");
	if(NULL == fp){

		printf("write_debug_file:open debug file failed<%s>,errno=%d\n", filename,errno);
		return -1;
	}

	fseek(fp, 0, SEEK_END);
	ui_size = ftell(fp);
	if(MAX_DEBUG_FILE_SIZE <= ui_size)
	{
		fclose(fp);
		/*增加日志文件后缀名*/
		strcat((char *)file_path, (char *)filename);
		strcat((char *)file_path, ".bak");
		rename((char *)filename, (char *)file_path);
		fp = fopen((char *)filename, "a+");
		if(!fp){
			return -1;
		}
	}

	time(&timep);
	localtime_r(&timep, &st_tm);
	sprintf((char *)log_time, "%02d-%02d-%02d %02d:%02d:%02d  ",
		(1900 + st_tm.tm_year),
		(1 + st_tm.tm_mon),
		st_tm.tm_mday,
		st_tm.tm_hour,
		st_tm.tm_min,
		st_tm.tm_sec);

	// 写入到文件中
	fwrite((char *)log_time, strlen((char *)log_time), 1, fp) ;
	fwrite((char *)buffer, strlen((char *)buffer), 1, fp) ;

	fclose(fp);
	return 0;
}

