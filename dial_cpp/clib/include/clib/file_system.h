/*
 * file_system.h
 *
 *  Created on: 2012-8-13
 *      Author: Administrator
 */

#ifndef CLIB_FILE_SYSTEM_H_
#define CLIB_FILE_SYSTEM_H_

#include <clib/array.h>

typedef bool (*dir_filter)(const char* dir, const char* subdir);
typedef bool (*file_filter)(const char* dir, const char* subdir);

/*
 *@brief        获取目录下子目录列表
 *@param in     dir         目录名称
 *@param out    sub_dirs    获取的子目录列表追加到该数组
 *@param in     filter      过滤程序，如果过滤程序返回false，则将该子目录过滤掉，不放入返回列表中
 *@return       返回-1表示出错
 */
int subdirs(const char* dir, str_array_t *sub_dirs, dir_filter filter);

/*
 *@brief        获取目录下子文件列表
 *@param in     dir         目录名称
 *@param out    files       获取的文件列表追加到该数组
 *@param in     filter      过滤程序，如果过滤程序返回false，则将该文件过滤掉，不放入返回列表中
 *@return       返回-1表示出错
 */
int subfiles(const char* dir, str_array_t *files, file_filter filter);
int subfiles_order_by_name(const char* dir, str_array_t *files,
        file_filter filter);


/*
 *@brief        获取目录下子文件列表，并按照名称进行排序
 *@param in     dir         目录名称
 *@param out    sub_dirs    获取的文件列表追加到该数组
 *@param in     filter      过滤程序，如果过滤程序返回false，则将该文件过滤掉，不放入返回列表中
 *@return       返回-1表示出错
 */
int subdirs_order_by_name(const char* dir, str_array_t *sub_dirs, dir_filter filter);

/*
 *@brief        获取目录剩余空间大小
 *@param in     dir         目录名称
 *@return       目录剩余空间大小（M），返回-1表示出错
 */
int free_space(const char* dir);

#endif /* CLIB_FILE_SYSTEM_H_ */
