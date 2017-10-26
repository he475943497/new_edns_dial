/**
 *Author:           wuyangchun
 *Date:             2012-07-26
 *Description:      常见文件读写操作
 *
 *Notice:           只是方便使用，不保证效率
 **/


#ifndef CLIB_FILE_H_
#define CLIB_FILE_H_

#include <clib/array.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *@brief        读取文件中的每行数据
 *@param in     filename    文件名称
 *@param in/out array       存放从文件中读取的每行字符串
 *@return       -1失败，其他成功
 *
 *@notice       不要用该函数读取大的文件，占用内存会很高，并且效率差
 *@notice       返回的字符串已经经过strtrim处理，去掉了开始和结束处的空格回车
 */
extern int file_readlines(const char *filename, str_array_t *array);

/*
 *@brief        将数据写入文件中(先清空原来的文件，再写数据)
 *@param in     filename    文件名称
 *@param in/out array       存放要写入的字符串
 *@return       -1失败，其他成功
 *
 */
extern int file_writelines(const char *filename, str_array_t *array);

/*
 *@brief        将数据追加到文件中
 *@param in     filename    文件名称
 *@param in/out array       存放要写入的字符串
 *@return       -1失败，其他成功
 *
 */
extern int file_appendline(const char *filename, const char* line);

/*
 *@brief        清空文件
 *@param in     filename    文件名称
 *@return       -1失败，其他成功
 *
 */
extern int file_clear(const char *filename);

#ifdef __cplusplus
}
#endif
#endif /* CLIB_FILE_H_ */
