/**
 *Author:           wuyangchun
 *Date:             2012-06-01
 *Description:      日志处理.
 *                  根据日志级别仅输出低于初始日志级别低的日志
 *
 *Notice:           当日志级别低于LOG_LEVEL_DEBUG时，不记录源文件名称和行号
 *                  由于在各个线程中都有可能打印日志
 *                  所以这里的函数都是多线程安全的
 **/

#ifndef CLIB_LOG_H_
#define CLIB_LOG_H_

#include <clib/types.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct _log log_t;

//日志级别
enum log_level {
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARNING = 2,
    LOG_LEVEL_TEST = 3,
    LOG_LEVEL_INFO = LOG_LEVEL_TEST,    //INFO级别用来取代TEST级别，后面将不再使用LOG_LEVEL_TEST
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_TRACE = 5
};

/**
 * 全局默认日志对象，默认输出到标准输出，日志级别为LOG_LEVEL_TRACE
 * 进程可以修改该日志对象,示例如下
 * if(g_default_log != NULL){
 *      log_free(g_default_log);
 * }
 * g_default_log = log_new_file(LOG_LEVEL_WARNING, "test");
 **/
extern log_t* g_default_log;

/*
 *@brief        创建标准输出日志对象
 *@param in     level    日志级别
 *@return       NULL表示失败
 */
log_t* log_new_stdout(enum log_level level);

/*
 *@brief        创建文件日志对象
 *@param in     level       日志级别
 *@param in     filename    日志文件名称
 *@return       NULL表示失败
 */
log_t* log_new_file(enum log_level level, const char *filename);

/*
 *@brief        创建自动控制大小的文件日志对象
 *@param in     level       日志级别
 *@param in     filename    日志文件名称
 *@param in     file_amount 日志文件个数，超过指定的个数后，会自动删除第一个日志文件
 *@param in     size        最大文件大小,单位K
 *@return       NULL表示失败
 */
log_t* log_new_rolling_file(enum log_level level, const char *filename,
        uint16_t file_amount, uint size);

void log_set_level(log_t* log, enum log_level level);
enum log_level log_level(log_t* log);

/*
 *@brief        释放日志对象
 *@param in     log    日志对象
 *@return
 */
void log_free(log_t* log);

/*
 *@brief        记录fmt参数指定的日志
 *@param in     log         日志对象
 *@param in     source_file 源代码文件名称
 *@param in     line        源代码所在行号
 *@param in     level       日志级别
 *@param in     fmt         日志内容
 *@return
 */
void log_str_fmt(log_t* log, const char* source_file, int line,
        enum log_level level, const char* fmt, ...);

void log_str(log_t* log, const char* source_file, int line,
        enum log_level level, const char* str);

/*
 *@brief        记录fmt参数指定的日志，并记录errno对应的错误信息
 *@param in     log         日志对象
 *@param in     source_file 源代码文件名称
 *@param in     line        源代码所在行号
 *@param in     level       日志级别
 *@param in     fmt         日志内容
 *@return
 */
void log_errno(log_t* log, const char* source_file, int line,
        enum log_level level, const char* fmt, ...);

//常用的宏定义
#define LOG(level, fmt, a...) log_str_fmt(g_default_log, __FILE__, __LINE__, level, fmt, ##a)

#define LOG_ERRNO(level, fmt, a...) log_errno(g_default_log, __FILE__, __LINE__, level, fmt, ##a)

//记录日志并且调用return;
#define LOG_IF_ERR(ret, level,  fmt, a...)   if(ret == -1){ LOG(level, fmt, ##a);}

//记录日志并且调用return;
#define LOG_RET(level,  fmt, a...)  { LOG(level, fmt, ##a);\
                return;}

#define LOG_RET_ZERO(level,  fmt, a...)  { LOG(level, fmt, ##a);\
                return 0;}

//记录日志并且调用return;
#define LOG_RET_ERR(level,  fmt, a...)  { LOG(level, fmt, ##a);\
                return -1;}

//如果ret==NULL，记录日志, 调用return;
#define LOG_RET_IF_NULL(ret, level, fmt, a...) if (ret == NULL){\
                LOG_RET(level, fmt, ##a);\
                }

#define LOG_RET_IF_ERR(ret, level, fmt, a...) if (ret == -1){\
                LOG_RET(level, fmt, ##a);\
                }

#define LOG_RET_ZERO_IF_ERR(ret, level, fmt, a...) if (ret == -1){\
		        LOG_RET_ZERO(level, fmt, ##a);\
                }

#define LOG_RET_ERR_IF_ERR(ret, level, fmt, a...) if (ret == -1){\
		        LOG_RET_ERR(level, fmt, ##a);\
                }

#define LOG_RET_ERR_IF_NULL(ret, level, fmt, a...) if (ret == NULL){\
                LOG_RET_ERR(level, fmt, ##a);\
                }

#define LOG_RET_ERR_IF_ZERO(ret, level, fmt, a...) if (ret == 0){\
        LOG_RET_ERR(level, fmt, ##a);\
        }

#define LOG_RET_ERR_IF_FALSE(ret, level, fmt, a...) if (ret == false){\
                LOG_RET_ERR(level, fmt, ##a);\
                }
//记录日志并且调用return;
#define LOG_ERRNO_RET(level,  fmt, a...) {LOG_ERRNO(level, fmt, ##a);\
                return;}

#define LOG_ERRNO_RET_ERR(level, fmt, a...) {LOG_ERRNO(level, fmt, ##a);\
                return -1;}

#define LOG_ERRNO_RET_ZERO(level, fmt, a...) {LOG_ERRNO(level, fmt, ##a);\
                return 0;}

#define LOG_ERRNO_RET_NULL(level, fmt, a...) {LOG_ERRNO(level, fmt, ##a);\
                return NULL;}

//如果ret==NULL，记录日志和errno对应的信息, 调用return;
#define LOG_ERRNO_RET_IF_NULL(ret, level, fmt, a...) if (ret == NULL){\
		        LOG_ERRNO_RET(level, fmt, ##a);\
            }

#define LOG_ERRNO_RET_ERR_IF_NULL(ret, level, fmt, a...) if (ret == NULL){\
                LOG_ERRNO_RET_ERR(level, fmt, ##a);\
            }

#define LOG_ERRNO_RET_ERR_IF_NOT_ZERO(ret, level, fmt, a...) if (ret != 0){\
                LOG_ERRNO_RET_ERR(level, fmt, ##a);\
            }

#define LOG_ERRNO_RET_NULL_IF_NULL(ret, level, fmt, a...) if (ret == NULL){\
                LOG_ERRNO_RET_NULL(level, fmt, ##a);\
            }

#define LOG_ERRNO_IF_ERR(ret, level, fmt, a...) if (ret == -1){\
                LOG_ERRNO(level, fmt, ##a);\
            }

#define LOG_ERRNO_RET_ZERO_IF_ERR(ret, level, fmt, a...) if (ret == -1){\
                LOG_ERRNO(level, fmt, ##a);\
                return 0;\
            }

#define LOG_ERRNO_RET_ERR_IF_ERR(ret, level, fmt, a...) if (ret == -1){\
                LOG_ERRNO(level, fmt, ##a);\
                return -1;\
            }
#define LOG_ERRNO_RET_FALSE_IF_ERR(ret, level, fmt, a...) if (ret == -1){\
                LOG_ERRNO(level, fmt, ##a);\
                return false;\
            }
#define LOG_ERRNO_RET_FALSE_IF_NULL(ret, level, fmt, a...) if (ret == NULL){\
        LOG_ERRNO(level, fmt, ##a);\
        return false;\
    }
#ifdef __cplusplus
}
#endif
#endif /* CLIB_LOG_H_ */
