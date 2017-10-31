/**
 *Author:           wuyangchun
 *Date:             2012-06-01
 *Description:      时间相关函数
 *
 *Notice:
 **/

#ifndef CLIB_TIME_H_
#define CLIB_TIME_H_

#include <time.h>
#include <clib/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TIME_STR_LEN (20)
/*
 *@brief        将时间格式化为可读的字符串（格式：2012-06-04 12:32:12）
 *@param in     time    待格式化的时间
 *@param out    buf     存放格式化结果
 *@param in     len     buf的长度
 *@return
 */
void time_format(time_t time, char *buf, uint len);

  /*@brief     计算一个unix时间戳所在分钟内的开始Unix时间戳
   *@param in  time     给定的Unix时间戳
   */
static inline time_t time_start_minute(time_t time)
{
    return time - (time % 60);
}

  /*@brief     计算一个unix时间戳所在分钟的下一个分钟的开始Unix时间戳
   *@param in  time     给定的Unix时间戳
   */
static inline time_t time_next_minute(time_t time)
{
    return time + (60 - time % 60);
}

  /*@brief     计算一个运行主机当前时间戳所在分钟内的开始Unix时间戳
   */
static inline time_t time_now_start_minute()
{
    return time_start_minute(time(NULL));
}
  /*@brief     计算一个运行主机当前时间戳所在分钟的下一分钟Unix时间戳
   */
static inline time_t time_now_next_minute()
{
    return time_next_minute(time(NULL));
}


#ifdef __cplusplus
}
#endif

#endif /* CLIB_TIME_H_ */
