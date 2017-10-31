/*
 * file.c
 *
 *  Created on: 2012-6-26
 *      Author: Administrator
 */
#include <clib/file.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <clib/macros.h>
#include <clib/string.h>
#include <clib/log.h>
#include <clib/memory.h>
#include <clib/array.h>

//int file_readlines(const char *filename, str_array_t *array)
//{
//    if (filename == NULL || array == NULL) {
//        return -1;
//    }
//
//    FILE *fp;
//    int filenameLen = strlen(filename);
//    char buffer[MAX_LINE_LEN];
//
//    if (filenameLen >= MAX_PATH_NAME_LEN) {
//        LOG(LOG_LEVEL_ERROR, "%s - 文件名称超过最大限制(%d)\n",
//                filename, MAX_PATH_NAME_LEN);
//        return -1;
//    }
//
//    fp = fopen(filename, "r");
//    LOG_ERRNO_RET_ERR_IF_NULL(fp, LOG_LEVEL_ERROR, "%s", filename);
//
//    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
//        char *ptr_buffer = strtrim(buffer, strlen(buffer));
//        if (*ptr_buffer != '\0') { //不是空字符串
//            char *value = (char*) mem_alloc(strlen(ptr_buffer) + 1);
//            strcpy(value, ptr_buffer);
//
//            str_array_append(array, value);
//        }
//    }
//
//    fclose(fp);
//
//    return 0;
//}

int file_readlines(const char *filename, str_array_t *array)
{
    if (filename == NULL || array == NULL) {
        return -1;
    }

    FILE *fp;
    int filenameLen = strlen(filename);

    if (filenameLen >= MAX_PATH_NAME) {
        LOG(LOG_LEVEL_ERROR, "%s - 文件名称超过最大限制(%d)\n",
                filename, MAX_PATH_NAME);
        return -1;
    }

    fp = fopen(filename, "r");
    LOG_ERRNO_RET_ERR_IF_NULL(fp, LOG_LEVEL_ERROR, "%s", filename);

    int ret = fseek(fp, 0, SEEK_END);
    if (ret == -1) {
        LOG_ERRNO(LOG_LEVEL_ERROR, "fseek失败");
        fclose(fp);
        return -1;
    }
    long file_size = ftell(fp);
    rewind(fp);


    char* buffer = mem_alloc(file_size+1);
    if (buffer == NULL) {
        LOG(LOG_LEVEL_ERROR, "分配内存失败%ld", file_size);
        fclose(fp);
        return -1;
    }
    size_t size = fread(buffer, 1, file_size, fp);
    if (size != file_size) {
        LOG_ERRNO(LOG_LEVEL_ERROR, "读取文件失败:%s", filename);
        mem_free(buffer);
        fclose(fp);
        return -1;
    }
    buffer[file_size] = 0;
    str_array_t *orig_array = strsplit(buffer, "\n");
    mem_free(buffer);
    for (int i = 0; i < str_array_size(orig_array); i++) {
        char *orig_str = str_array_at(orig_array, i);
        char *ptr_buffer = strtrim(orig_str, strlen(orig_str));
        if (*ptr_buffer != '\0') { //不是空字符串
            char *value = (char*) mem_alloc(strlen(ptr_buffer) + 1);
            strcpy(value, ptr_buffer);

            str_array_append(array, value);
        }
    }

    str_array_free(orig_array);

    fclose(fp);

    return 0;
}

int file_writelines(const char *filename, str_array_t *array)
{
    if (filename == NULL || array == NULL) {
        return -1;
    }

    FILE *fp;
    int filenameLen = strlen(filename);
    int array_len = str_array_size(array);
    int ret;

    if (filenameLen >= MAX_PATH_NAME) {
        LOG(LOG_LEVEL_ERROR, "%s - 文件名称超过最大限制(%d)\n",
                filename, MAX_PATH_NAME);
        return -1;
    }

    fp = fopen(filename, "w+");
    LOG_ERRNO_RET_ERR_IF_NULL(fp, LOG_LEVEL_ERROR, "%s", filename);

    for (int i = 0; i < array_len; i++) {
        ret = fprintf(fp, "%s\n", str_array_at(array, i));
        if (ret < 0) {
            LOG_ERRNO(LOG_LEVEL_ERROR, "%s", filename);
            fclose(fp);
            return -1;
        }
    }

    fclose(fp);

    return 0;
}

int file_appendline(const char *filename, const char* line)
{
    if (filename == NULL || line == NULL) {
        return -1;
    }

    FILE *fp;
    int filenameLen = strlen(filename);
    int ret;

    if (filenameLen >= MAX_PATH_NAME) {
        LOG(LOG_LEVEL_ERROR, "%s - 文件名称超过最大限制(%d)\n",
                filename, MAX_PATH_NAME);
        return -1;
    }

    fp = fopen(filename, "a+");
    LOG_ERRNO_RET_ERR_IF_NULL(fp, LOG_LEVEL_ERROR, "%s", filename);

    ret = fprintf(fp, "%s\n", line);
    if (ret < 0) {
        LOG_ERRNO(LOG_LEVEL_ERROR, "%s", filename);
        fclose(fp);
        return -1;
    }

    fclose(fp);

    return 0;
}

int file_clear(const char *filename)
{
    FILE *fp = fopen(filename, "w+");
    LOG_ERRNO_RET_ERR_IF_NULL(fp, LOG_LEVEL_ERROR, "%s", filename);

    fclose(fp);

    return 0;
}
