/*
 * 由于在各个线程中都有可能打印日志，所以日志相关的操作需要线程安全，这部分还未实现
 */
#include <clib/log.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <clib/time.h>
#include <unistd.h>
#include <clib/memory.h>
#include <pthread.h>

#define LOCAL_LOG(level, fmt, a...) local_log(__FILE__, __LINE__,  level, fmt, ##a)

log_t* g_default_log = NULL;
static pthread_mutex_t l_default_log_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t l_local_log_mutex = PTHREAD_MUTEX_INITIALIZER;

#define MAX_LOG_LENGTH 2048

typedef void (*log_func)(void* log_param, const char* loginfo);
typedef void (*log_destory_func)(void* log_param);

struct _log {
    void* log_param;
    log_func log;
    int level;
    log_destory_func log_destory;
    pthread_mutex_t mutex;
};

struct rolling_file_param {
    FILE* fd;                                     //文件句柄
    uint16_t current_file_number;                  //当前到第几个文件
    uint16_t max_file_number;                      //最大文件计数
    uint max_size;                                //最大文件规格
    char file_name[MAX_PATH_NAME];            //文件名称
};

static void local_log(const char* source_file, int line, enum log_level level,
        const char* fmt, ...);

static void log_stdout(void *param, const char* loginfo);

static void* log_file_new(const char *filename);
static void log_file(void *param, const char* loginfo);
static void log_file_destory(void *param);

static void* log_rolling_file_new(const char *filename, uint16_t file_amount,
        uint size);
static void log_rolling_file(void *param, const char* loginfo);
static void log_rolling_file_destory(void *param);

void log_set_level(log_t* log, enum log_level level)
{
    if (log == NULL) {
        return;
    }

    struct _log *_log = (struct _log *) log;
    pthread_mutex_lock(&_log->mutex);
    _log->level = level;
    pthread_mutex_unlock(&_log->mutex);
}

log_t* log_new_stdout(enum log_level level)
{
    struct _log *_log = (struct _log*) mem_alloc(sizeof(struct _log));

    _log->log_param = NULL;
    _log->log = log_stdout;
    _log->level = level;
    _log->log_destory = NULL;
    pthread_mutex_init(&_log->mutex, NULL);

    return _log;
}

log_t* log_new_file(enum log_level level, const char *filename)
{
    void *log_param;
    struct _log *_log;

    log_param = log_file_new(filename);
    if (log_param == NULL) {
        return NULL;
    }
    _log = (struct _log*) mem_alloc(sizeof(struct _log));

    _log->log_param = log_param;
    _log->log = log_file;
    _log->level = level;
    _log->log_destory = log_file_destory;
    pthread_mutex_init(&_log->mutex, NULL);

    return _log;
}

log_t* log_new_rolling_file(enum log_level level, const char *filename,
        uint16_t file_amount, uint size)
{
    void *log_param;
    struct _log *_log;

    log_param = log_rolling_file_new(filename, file_amount, size);
    if (log_param == NULL) {
        return NULL;
    }
    _log = (struct _log*) mem_alloc(sizeof(struct _log));

    _log->log_param = log_param;
    _log->log = log_rolling_file;
    _log->level = level;
    _log->log_destory = log_rolling_file_destory;
    pthread_mutex_init(&_log->mutex, NULL);

    return _log;
}

void log_free(log_t* log)
{
    struct _log *_log = (struct _log *) log;
    if (_log->log_param != NULL) {
        _log->log_destory(_log->log_param);
    }
    pthread_mutex_destroy(&_log->mutex);
    mem_free(_log);
}

static void create_default_log()
{
    if (g_default_log == NULL) {
        g_default_log = log_new_stdout(LOG_LEVEL_TRACE);
    }
}

static bool prepare_log(struct _log *_log, const char* source_file, int line,
        enum log_level level, char* buf, uint buflen)
{
    int tmp_buf_len;

    if (_log == NULL) {
        return false;
    }

    time_format(time(NULL), buf, buflen);
    tmp_buf_len = strlen(buf);
    switch (level) {
    case LOG_LEVEL_TRACE:
        strncat(buf, " [TRACE] ", buflen - tmp_buf_len);
        break;
    case LOG_LEVEL_DEBUG:
        strncat(buf, " [DEBUG] ", buflen - tmp_buf_len);
        break;
    case LOG_LEVEL_TEST:
        strncat(buf, " [INFO] ", buflen - tmp_buf_len);
        break;
    case LOG_LEVEL_WARNING:
        strncat(buf, " [WARN] ", buflen - tmp_buf_len);
        break;
    case LOG_LEVEL_ERROR:
        strncat(buf, " [ERROR] ", buflen - tmp_buf_len);
        break;
    default:
        break;
    }
    buf[buflen - 1] = '\0';
    tmp_buf_len = strlen(buf);
    if (_log->level > LOG_LEVEL_TEST) {
        snprintf(buf + tmp_buf_len, buflen - tmp_buf_len, "%s(%d) - ",
                source_file, line);
        buf[buflen - 1] = '\0';
    }

    return true;
}

static void local_log(const char* source_file, int line, enum log_level level,
        const char* fmt, ...)
{
    va_list vap;
    char log_buf[MAX_LOG_LENGTH];
    int buflen;
    struct _log log;

    bzero(&log, sizeof(struct _log));
    log.level = level;

    if (!prepare_log(&log, source_file, line, level, log_buf,
            sizeof(log_buf))) {
        return;
    }

    buflen = strlen(log_buf);
    va_start(vap, fmt);
    vsnprintf(log_buf + buflen, sizeof(log_buf) - buflen, fmt, vap);
    log_buf[sizeof(log_buf) - 1] = '\0';
    va_end(vap);

    pthread_mutex_lock(&l_local_log_mutex);
    log_stdout(NULL, log_buf);
    pthread_mutex_unlock(&l_local_log_mutex);
}

void log_str_fmt(log_t* log, const char* source_file, int line,
        enum log_level level, const char* fmt, ...)
{

    struct _log *_log = (struct _log *) log;

    va_list vap;
    char log_buf[MAX_LOG_LENGTH];
    int buflen;

    if (_log == NULL) {
        pthread_mutex_lock(&l_default_log_mutex);
        create_default_log();
        _log = g_default_log;
        pthread_mutex_unlock(&l_default_log_mutex);
    }

    if (level > _log->level) {
        return;
    }

    pthread_mutex_lock(&_log->mutex);
    if (!prepare_log(_log, source_file, line, level, log_buf,
            sizeof(log_buf))) {
        pthread_mutex_unlock(&_log->mutex);
        return;
    }

    buflen = strlen(log_buf);
    va_start(vap, fmt);
    vsnprintf(log_buf + buflen, sizeof(log_buf) - buflen, fmt, vap);
    log_buf[sizeof(log_buf) - 1] = '\0';
    va_end(vap);
    _log->log(_log->log_param, log_buf);
    pthread_mutex_unlock(&_log->mutex);

}

void log_str(log_t* log, const char* source_file, int line,
        enum log_level level, const char* str)
{
    struct _log *_log = (struct _log *) log;

    char log_buf[MAX_LOG_LENGTH];
    int buflen;

    if (_log == NULL) {
        pthread_mutex_lock(&l_default_log_mutex);
        create_default_log();
        _log = g_default_log;
        pthread_mutex_unlock(&l_default_log_mutex);
    }

    if (level > _log->level) {
        return;
    }

    pthread_mutex_lock(&_log->mutex);
    if (!prepare_log(_log, source_file, line, level, log_buf,
            sizeof(log_buf))) {
        pthread_mutex_unlock(&_log->mutex);
        return;
    }

    buflen = strlen(log_buf);
    strncpy(log_buf + buflen, str, sizeof(log_buf) - buflen);
    log_buf[sizeof(log_buf) - 1] = '\0';
    _log->log(_log->log_param, log_buf);
    pthread_mutex_unlock(&_log->mutex);
}
void log_errno(log_t* log, const char* source_file, int line,
        enum log_level level, const char* fmt, ...)
{
    struct _log *_log = (struct _log *) log;
    va_list vap;
    char log_buf[MAX_LOG_LENGTH];
    int buflen;
    char errno_str[MAX_LOG_LENGTH];

    strerror_r(errno, errno_str, sizeof(errno_str));
    errno_str[sizeof(errno_str) - 1] = '\0';

    if (_log == NULL) {
        pthread_mutex_lock(&l_default_log_mutex);
        create_default_log();
        _log = g_default_log;
        pthread_mutex_unlock(&l_default_log_mutex);
    }

    if (level > _log->level) {
        return;
    }

    pthread_mutex_lock(&_log->mutex);
    if (!prepare_log(_log, source_file, line, level, log_buf,
            sizeof(log_buf))) {
        pthread_mutex_unlock(&_log->mutex);
        return;
    }

    buflen = strlen(log_buf);
    va_start(vap, fmt);
    vsnprintf(log_buf + buflen, sizeof(log_buf) - buflen, fmt, vap);
    log_buf[sizeof(log_buf) - 1] = '\0';
    va_end(vap);

    buflen = strlen(log_buf);
    snprintf(log_buf + buflen, sizeof(log_buf) - buflen, " (%s)", errno_str);
    log_buf[sizeof(log_buf) - 1] = '\0';

    _log->log(_log->log_param, log_buf);
    pthread_mutex_unlock(&_log->mutex);
}

/*
 *  stdout log
 */
static void log_stdout(void *param, const char* loginfo)
{
    printf("%s\n", loginfo);
}

/*
 *  file log
 */
static void* log_file_new(const char *filename)
{
    FILE *file = fopen(filename, "a");

    if (file == NULL) {
        LOCAL_LOG(LOG_LEVEL_ERROR, "打开日志文件失败%s:", filename);
    }

    return file;
}

static void log_file_destory(void *param)
{
    fclose((FILE*) param);
}

static void log_file(void *param, const char* loginfo)
{
    fprintf((FILE*) param, "%s\n", loginfo);
}

/*
 *  rolling file log
 */
static void* log_rolling_file_new(const char *filename, uint16_t file_amount,
        uint size)
{
    struct rolling_file_param *param;
    int i;
    int fd_exist;
    char tmp_buf[255];

    FILE *file = fopen(filename, "a");

    if (file == NULL) {
        LOCAL_LOG(LOG_LEVEL_ERROR, "打开日志文件失败%s:", filename);
        return NULL;
    }

    param = (struct rolling_file_param*) mem_alloc(
            sizeof(struct rolling_file_param));
    param->fd = file;
    param->max_file_number = file_amount;
    param->max_size = size;
    strcpy(param->file_name, filename);

    //判断当前已经到第几个文件了
    for (i = 1; i <= param->max_file_number; i++) {
        snprintf(tmp_buf, sizeof(tmp_buf), "%s%c%d", filename, '_', i);
        if ((fd_exist = access(tmp_buf, F_OK)) != 0) {
            break;
        }
    }
    param->current_file_number = i;

    return param;

}

static void log_rolling_file_destory(void *param)
{
    struct rolling_file_param *_param = (struct rolling_file_param *) param;
    fclose(_param->fd);
    mem_free(_param);
}

static void log_rolling_file(void *param, const char* loginfo)
{
    int fl_len = 0;
    char new_file_name[MAX_PATH_NAME];
    char new_file_tmp[MAX_PATH_NAME];
    char file_name_buf[MAX_PATH_NAME];
    char file_name_buf0[MAX_PATH_NAME];

    int j;

    struct rolling_file_param *_param = (struct rolling_file_param *) param;

    if (-1 == fprintf(_param->fd, "%s\n", loginfo)) {
        LOCAL_LOG(LOG_LEVEL_ERROR, "写入日志文件失败%s:", _param->file_name);
        return;
    }
    fl_len = ftell(_param->fd);

    if (fl_len < _param->max_size * 1024) {
        return;
    }

    if (_param->current_file_number > _param->max_file_number) {
        for (j = 0; j < _param->max_file_number; j++) {
            snprintf(new_file_name, sizeof(new_file_name), "%s%c%d",
                    _param->file_name, '_', j);
            snprintf(new_file_tmp, sizeof(new_file_tmp), "%s%c%d",
                    _param->file_name, '_', j + 1);
            if (0 != rename(new_file_tmp, new_file_name)) {
                LOCAL_LOG(LOG_LEVEL_ERROR, "重命名文件失败%s:", new_file_tmp);
                return;
            }
        }
        snprintf(file_name_buf0, sizeof(file_name_buf0), "%s_0",
                _param->file_name);
        if (0 != remove(file_name_buf0)) {
            LOCAL_LOG(LOG_LEVEL_ERROR, "删除日志文件失败%s:", file_name_buf0);
            return;
        }

        _param->current_file_number = _param->max_file_number;

    }

    if (0 != fclose(_param->fd)) {
        LOCAL_LOG(LOG_LEVEL_ERROR, "关闭日志文件失败%s:", _param->file_name);
        return;
    }

    snprintf(file_name_buf, sizeof(file_name_buf), "%s%c%d", _param->file_name,
            '_', _param->current_file_number);

    if (0 != rename(_param->file_name, file_name_buf)) {
        LOCAL_LOG(LOG_LEVEL_ERROR, "重命名文件失败%s:", file_name_buf);
        return;
    }

    _param->fd = fopen(_param->file_name, "a");

    if (_param->fd == NULL) {
        LOCAL_LOG(LOG_LEVEL_ERROR, "打开日志文件失败%s:", _param->file_name);
        return;
    }

    _param->current_file_number++;

}


enum log_level log_level(log_t* log)
{
    return log->level;
}
