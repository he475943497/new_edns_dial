/**
 *Author:           wuyangchun
 *Date:             2012-07-26
 *Description:      常用宏定义
 *
 *Notice:
 **/

#ifndef CLIB_MACROS_H_
#define CLIB_MACROS_H_

#include<linux/limits.h>

#ifndef MAX_FILE_NAME
#define MAX_FILE_NAME NAME_MAX           //最大文件名称长度
#endif

#ifndef MAX_PATH_NAME
#define MAX_PATH_NAME PATH_MAX          //最大文件路径长度
#endif

#ifndef MAX_LINEs
#define MAX_LINE 1024                   //最大行长度
#endif

#define RETURN_VAL_IF_FAIL(expr, val) {if(!(expr)) return val;}
#define RETURN_IF_FAIL(expr) {if(!(expr)) return;}

#ifndef MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#endif

#ifndef ABS
#define ABS(a)     (((a) < 0) ? -(a) : (a))
#endif

#endif /* CLIB_MACROS_H_ */
