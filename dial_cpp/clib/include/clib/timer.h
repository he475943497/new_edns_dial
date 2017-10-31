/**
 *Author:           wanghonghui
 *Date:             2012-06-06
 *Description:      定时器，提供秒级的定时器
 *
 **/
#ifndef CLIB_TIMER_H_
#define CLIB_TIMER_H_

#include <signal.h>
#include <time.h>
#include <clib/types.h>


#ifdef __cplusplus
extern "C" {
#endif

/*union sigval 结构：
 *union sigval {                          Data passed with notification 
 *           int     sival_int;           Integer value 
 *           void   *sival_ptr;           Pointer value (timer_new中的void *data指针会赋给这个指针）
 *      };
 */

typedef void (*timer_handler)(union sigval);

/*
 *@param in     timer_id    定时器结构指针
 *@param in     func        回调函数
 *@param in     data        回调函数的参数
 *@return       0：成功 
 *              错误码：失败  
 *  错误码如下:
 *	EINVAL
 *             An invalid which_clock value was specified.
 *  EAGAIN
 *             The system could not process the request.
 *  EFAULT
 *             An invalid timer_event_spec value was specified.
 */
int timer_new(timer_t *timer_id, timer_handler func, void *data);


/*
 *@brief        启动定时器
 *@param in     timer_id    定时器
 *@param in     interval    定时器间隔（秒）
 *@param in     nano_sec    秒级以下定时，纳秒(nano second), 上面interval参数为0
 *@return       0:成功 
 *              错误码：失败 
 * 错误码如下:
 *  EFAULT 
 *	        new_value, old_value, or curr_value is not a valid pointer.
 *  EINVAL 
 *	        timerid is invalid.
 *          timer_settime() may fail with the following errors:
 *  EINVAL    
 *	        new_value.it_value is negative; or new_value.it_value.tv_nsec is
 *          negative or greater than 999,999,999.
 */
 
int timer_start(timer_t timer_id, uint interval, long nano_sec);
int timer_oneshot(timer_t timer_id, uint interval, long nano_sec);

/*
 *@brief        停止定时器（停止之后可以再次启动）
 *@param in     timer_id    定时器
 *@return       0:成功 
 *              错误码：失败 
 * 错误码如下:
 *  EFAULT 
 *	        new_value, old_value, or curr_value is not a valid pointer.
 *  EINVAL 
 *	        timerid is invalid.
 *          timer_settime() may fail with the following errors:
 *  EINVAL    
 *	        new_value.it_value is negative; or new_value.it_value.tv_nsec is
 *          negative or greater than 999,999,999.
 */
int timer_stop(timer_t timer_id);

/*
 *@brief        释放定时器
 *@param in     timer    定时器
 *@return       0:成功 
 *              错误码：失败
 * 错误码如下：
 *  EINVAL 
 *          timerid is not a valid timer ID.
 */
int timer_free(timer_t timer_id);


#ifdef __cplusplus
}
#endif

#endif /* CLIB_TIMER_H_ */
