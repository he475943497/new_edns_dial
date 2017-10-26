/**
 *  该文件主要用来读写配置文件，一般情况下对性能要求不高，所以代码的性能没有可读性重要
 */

#include <clib/ini.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <clib/macros.h>
#include <clib/string.h>
#include <clib/log.h>
#include <clib/memory.h>
#include <clib/array.h>

#define MAX_INI_LINE_LEN 1024

struct _ini {
    char filename[MAX_PATH_NAME];
    FILE *fp;
};

ini_t* ini_new(const char *ini_file)
{

    struct _ini *ini;
    FILE *fp;
    int filenameLen = strlen(ini_file);

    if (filenameLen >= MAX_PATH_NAME) {
        LOG(LOG_LEVEL_ERROR, "%s - 文件名称超过最大限制(%d)\n", ini_file, MAX_PATH_NAME);
        return NULL;
    }

    fp = fopen(ini_file, "r+");

    LOG_ERRNO_RET_NULL_IF_NULL(fp, LOG_LEVEL_ERROR, "%s", ini_file);

    ini = (struct _ini*) mem_alloc(sizeof(struct _ini));
    bzero(ini, sizeof(struct _ini));
    memcpy(ini->filename, ini_file, filenameLen + 1);
    ini->fp = fp;

    return ini;
}

void ini_free(ini_t* ini)
{
    struct _ini *_ini = (struct _ini *) ini;
    fclose(_ini->fp);
    mem_free(ini);
}

bool ini_get_str(ini_t* ini, const char *section, const char *key, char *value,
        uint value_size, const char *def_val)
{
    str_array_t *values = str_array_new_full(1);
    bool ret = ini_get_str_array(ini, section, key, values);

    if (ret) { //找到就取第一个
        strncpy(value, str_array_at(values, 0), value_size);
        value[value_size - 1] = '\0';
        str_array_free(values);
        return true;
    } else {
        if (def_val == NULL) {
            value[0] = '\0';
        } else {
            strncpy(value, def_val, value_size);
            value[value_size - 1] = '\0';
        }
        str_array_free(values);
        return false;
    }
}

static long find_section_pos(ini_t* ini, const char *section)
{
    char buffer[MAX_INI_LINE_LEN];
    char *saveptr;
    char *ptr_buffer;
    char *mark1 = NULL, *mark2 = NULL;
    int readlen;

    struct _ini *_ini = (struct _ini *) ini;

    rewind(_ini->fp);
    while (fgets(buffer, sizeof(buffer), _ini->fp) != NULL) {
        readlen = strlen(buffer);
        ptr_buffer = strtrim(buffer, readlen);
        if (ptr_buffer[0] == ';') {   //注释
            continue;
        } else if (ptr_buffer[0] == '[') { //段
            mark1 = (char*) strtok_r(ptr_buffer, "[", &saveptr);
            mark2 = (char*) strtok_r(mark1, "]", &saveptr);
            if (mark2 == NULL) { //段格式不正确
                continue;
            }
            mark2 = strtrim(mark2, strlen(mark2));

            if (strcmp(mark2, section) == 0) {
                return ftell(_ini->fp) - readlen;
            }
        }
    }

    return -1;
}

static long find_next_section_pos(ini_t* ini, long section_pos)
{
    char buffer[MAX_INI_LINE_LEN];
    char *ptr_buffer;
    int readlen;
    int ret;

    struct _ini *_ini = (struct _ini *) ini;

    ret = fseek(_ini->fp, section_pos, SEEK_SET);
    LOG_ERRNO_RET_ERR_IF_ERR(ret, LOG_LEVEL_ERROR, "fseek失败");
    //第一行是多余的，所以先读出来
    if (fgets(buffer, sizeof(buffer), _ini->fp) == NULL) {
        return -1;
    }
    while (fgets(buffer, sizeof(buffer), _ini->fp) != NULL) {
        readlen = strlen(buffer);
        ptr_buffer = strtrim(buffer, readlen);
        if (ptr_buffer[0] == '[') { //段
            return ftell(_ini->fp) - readlen;
        }
    }

    return -1;
}

static long find_key_pos(ini_t* ini, long section_pos, const char *key)
{
    char buffer[MAX_INI_LINE_LEN];
    char *searchkey;
    char *saveptr;
    char *ptr_buffer;
    int readlen;
    int ret;

    struct _ini *_ini = (struct _ini *) ini;

    ret = fseek(_ini->fp, section_pos, SEEK_SET);
    LOG_ERRNO_RET_ERR_IF_ERR(ret, LOG_LEVEL_ERROR, "fseek失败");
    //第一行是多余的，所以先读出来
    if (fgets(buffer, sizeof(buffer), _ini->fp) == NULL) {
        return -1;
    }
    while (fgets(buffer, sizeof(buffer), _ini->fp) != NULL) {
        readlen = strlen(buffer);
        ptr_buffer = strtrim(buffer, readlen);
        if (ptr_buffer[0] == ';') {   //注释
            continue;
        } else if (ptr_buffer[0] == '[') { //段
            return -1;
        } else {    //key
            searchkey = (char*) strtok_r(ptr_buffer, "=", &saveptr);
            if (searchkey == NULL) {
                continue;
            }
            searchkey = strtrim(searchkey, strlen(searchkey));
            if (strcmp(searchkey, key) == 0) {
                return ftell(_ini->fp) - readlen;
            }

        }
    }
    return -1;
}

static long find_next_key_pos(ini_t* ini, long key_pos, const char *key)
{
    return find_key_pos(ini, key_pos, key);
}

bool ini_get_str_array(ini_t* ini, const char *section, const char *key,
        str_array_t* values)
{
    char buffer[MAX_INI_LINE_LEN];
    char *searchkey;
    char *searchvalue;
    char *saveptr;
    char *ptr_buffer;
    char *value;
    long section_pos;
    long key_pos;
    int readlen;
    int ret;

    struct _ini *_ini = (struct _ini *) ini;
    if (_ini->fp == NULL) {
        LOG(LOG_LEVEL_ERROR, "_ini->fp is NULL");
        return false;
    }

    rewind(_ini->fp);

    section_pos = find_section_pos(ini, section);
    if (section_pos == -1) {
        return false;
    }

    key_pos = find_key_pos(ini, section_pos, key);
    if (key_pos == -1) {
        return false;
    }

    do {
        ret = fseek(_ini->fp, key_pos, SEEK_SET);
        LOG_ERRNO_RET_FALSE_IF_ERR(ret, LOG_LEVEL_ERROR, "fseek失败");
        if (fgets(buffer, sizeof(buffer), _ini->fp) == NULL) {
            return -1;
        }
        readlen = strlen(buffer);
        ptr_buffer = strtrim(buffer, readlen);

        searchkey = (char*) strtok_r(ptr_buffer, "=", &saveptr);
        if (searchkey == NULL) {
            continue;
        }
        searchkey = strtrim(searchkey, strlen(searchkey));
        if (strcmp(searchkey, key) != 0)
            continue;
        searchvalue = (char*) strtok_r(NULL, "=", &saveptr);
        if (searchvalue == NULL) {
            continue;
        }
        searchvalue = strtrim(searchvalue, strlen(searchvalue));
        value = (char*) mem_alloc(strlen(searchvalue) + 1);
        strcpy(value, searchvalue);

        str_array_append(values, value);

    } while ((key_pos = find_next_key_pos(ini, key_pos, key)) != -1);

    if (str_array_size(values) > 0) {
        return true;
    }

    return false;
}
static bool ini_reaplace(ini_t *ini, long start_pos, long end_pos,
        const char* value)
{
    int ret;
    long file_size;
    long temp_len;
    char *temp_buf_pre = NULL;
    char *temp_buf_next = NULL;
    bool result = false;

    struct _ini *_ini = (struct _ini *) ini;

    ret = fseek(_ini->fp, 0, SEEK_END);
    LOG_ERRNO_RET_FALSE_IF_ERR(ret, LOG_LEVEL_ERROR, "fseek失败");
    file_size = ftell(_ini->fp);

    //取前半段
    if (start_pos > 0) {
        temp_buf_pre = mem_alloc(start_pos);
        ret = fseek(_ini->fp, 0, SEEK_SET);
        if (ret == -1) {
            LOG_ERRNO(LOG_LEVEL_ERROR, "fseek失败");
            goto ERR;
        }
        temp_len = fread(temp_buf_pre, 1, start_pos, _ini->fp);
        if (temp_len != (start_pos)) {
            LOG_ERRNO(LOG_LEVEL_ERROR, "fread失败");
            goto ERR;
        }
    }

    //取后半段
    if (end_pos != -1 && end_pos != file_size) {
        temp_buf_next = mem_alloc(file_size - end_pos);
        ret = fseek(_ini->fp, end_pos, SEEK_SET);
        if (ret == -1) {
            LOG_ERRNO(LOG_LEVEL_ERROR, "fseek失败");
            goto ERR;
        }
        temp_len = fread(temp_buf_next, 1, file_size - end_pos, _ini->fp);
        if (temp_len != (file_size - end_pos)) {
            LOG_ERRNO(LOG_LEVEL_ERROR, "fread失败");
            goto ERR;
        }
    }

    //清空文件
    fclose(_ini->fp);
    _ini->fp = fopen(_ini->filename, "w+");
    LOG_ERRNO_RET_FALSE_IF_NULL(_ini->fp, LOG_LEVEL_ERROR, "%s",
            _ini->filename);

    //写新的
    if (start_pos > 0) {
        temp_len = fwrite(temp_buf_pre, 1, start_pos, _ini->fp);
        if (temp_len != start_pos) {
            LOG_ERRNO(LOG_LEVEL_ERROR, "fwrite失败");
            goto ERR;
        }
    }
    if (value != NULL && value[0] != '\0') {
        if (fprintf(_ini->fp, "%s", value) < 0) {
            LOG_ERRNO(LOG_LEVEL_ERROR, "写%s失败", value);
            goto ERR;
        }
    }
    if (end_pos != -1 && end_pos != file_size) {
        temp_len = fwrite(temp_buf_next, 1, file_size - end_pos, _ini->fp);
        if (temp_len != file_size - end_pos) {
            LOG_ERRNO(LOG_LEVEL_ERROR, "fwrite失败");
            goto ERR;
        }
    }

    //重新打开文件
    fclose(_ini->fp);
    _ini->fp = fopen(_ini->filename, "r+");
    LOG_ERRNO_RET_FALSE_IF_NULL(_ini->fp, LOG_LEVEL_ERROR, "%s",
            _ini->filename);

    result = true;

    ERR: if (temp_buf_pre != NULL) {
        mem_free(temp_buf_pre);
    }
    if (temp_buf_next != NULL) {
        mem_free(temp_buf_next);
    }
    return result;
}

bool ini_set_str(ini_t* ini, const char *section, const char *key,
        const char *value)
{
    long key_pos;
    long section_pos;
    int ret;
    char buffer[MAX_INI_LINE_LEN];

    struct _ini *_ini = (struct _ini *) ini;
    if (_ini->fp == NULL) {
        LOG(LOG_LEVEL_ERROR, "_ini->fp is NULL");
        return false;
    }
    rewind(_ini->fp);

    section_pos = find_section_pos(ini, section);
    if (section_pos == -1) {    //没找到对应的section

        return ini_append_str(ini, section, key, value);
    }

    key_pos = find_key_pos(ini, section_pos, key);
    if (key_pos == -1) {    //没找到key
        return ini_append_str(ini, section, key, value);
    }

    ret = fseek(_ini->fp, key_pos, SEEK_SET);
    LOG_ERRNO_RET_FALSE_IF_ERR(ret, LOG_LEVEL_ERROR, "fseek失败");
    if (fgets(buffer, sizeof(buffer), _ini->fp) == NULL) {
        return false;
    }
    snprintf(buffer, sizeof(buffer), "%s=%s\n", key, value);

    return ini_reaplace(ini, key_pos, ftell(_ini->fp), buffer);
}

bool ini_append_str(ini_t* ini, const char *section, const char *key,
        const char *value)
{
    str_array_t *values = str_array_new_sized(1);
    str_array_append(values, value);
    bool ret = ini_append_str_array(ini, section, key, values);

    str_array_free(values);

    return ret;

}

bool ini_append_str_array(ini_t* ini, const char *section, const char *key,
        str_array_t* values)
{
    long section_pos;
    int ret;
    long file_size;
    long temp_len;
    int i;
    int values_len = str_array_size(values);

    struct _ini *_ini = (struct _ini *) ini;
    if (_ini->fp == NULL) {
        LOG(LOG_LEVEL_ERROR, "_ini->fp is NULL");
        return false;
    }

    ret = fseek(_ini->fp, 0, SEEK_END);
    LOG_ERRNO_RET_FALSE_IF_ERR(ret, LOG_LEVEL_ERROR, "fseek失败");
    file_size = ftell(_ini->fp);
    rewind(_ini->fp);

    section_pos = find_section_pos(ini, section);
    if (section_pos == -1) {    //没找到对应的section
        if (file_size > 0) {    //末尾添加换行
            char last_character[1] = { 0 };
            ret = fseek(_ini->fp, file_size - 1, SEEK_SET);
            if (ret == -1) {
                LOG_ERRNO(LOG_LEVEL_ERROR, "fseek失败");
                goto ERR;
            }
            fread(last_character, 1, 1, _ini->fp);
            if (last_character[0] != '\n') {
                fwrite("\n", 1, 1, _ini->fp);
            }
        }
        ret = fseek(_ini->fp, 0, SEEK_END);
        LOG_ERRNO_RET_FALSE_IF_ERR(ret, LOG_LEVEL_ERROR, "fseek失败");
        if (fprintf(_ini->fp, "[%s]\n", section) < 0) {
            LOG_ERRNO(LOG_LEVEL_ERROR, "写section(%s)失败", section);
            return false;
        }
        for (i = 0; i < values_len; i++) {
            char *value = str_array_at(values, i);
            if (fprintf(_ini->fp, "%s=%s\n", key, value) < 0) {
                LOG_ERRNO(LOG_LEVEL_ERROR, "写%s=%s失败", key, value);
                return false;
            }
        }
        return true;
    }

    section_pos = find_next_section_pos(ini, section_pos);
    if (section_pos == -1) {    //在文件尾添加
        if (file_size > 0) {    //末尾添加换行
            char last_character[1] = { 0 };
            ret = fseek(_ini->fp, file_size - 1, SEEK_SET);
            if (ret == -1) {
                LOG_ERRNO(LOG_LEVEL_ERROR, "fseek失败");
                goto ERR;
            }
            fread(last_character, 1, 1, _ini->fp);
            if (last_character[0] != '\n') {
                fwrite("\n", 1, 1, _ini->fp);
            }
        }
        ret = fseek(_ini->fp, 0, SEEK_END);
        LOG_ERRNO_RET_FALSE_IF_ERR(ret, LOG_LEVEL_ERROR, "fseek失败");
        for (i = 0; i < values_len; i++) {
            char *value = str_array_at(values, i);
            if (fprintf(_ini->fp, "%s=%s\n", key, value) < 0) {
                LOG_ERRNO(LOG_LEVEL_ERROR, "写%s=%s失败", key, value);
                return false;
            }
        }
        return true;
    }

    char *temp_buf = mem_alloc(file_size - section_pos);

    //取后半段
    ret = fseek(_ini->fp, section_pos, SEEK_SET);
    if (ret == -1) {
        LOG_ERRNO(LOG_LEVEL_ERROR, "fseek失败");
        goto ERR;
    }
    temp_len = fread(temp_buf, 1, file_size - section_pos, _ini->fp);
    if (temp_len != (file_size - section_pos)) {
        LOG_ERRNO(LOG_LEVEL_ERROR, "fread失败");
        goto ERR;
    }

    //写新的
    ret = fseek(_ini->fp, section_pos, SEEK_SET);
    if (ret == -1) {
        LOG_ERRNO(LOG_LEVEL_ERROR, "fseek失败");
        goto ERR;
    }
    for (i = 0; i < values_len; i++) {
        char *value = str_array_at(values, i);
        if (fprintf(_ini->fp, "%s=%s\n", key, value) < 0) {
            LOG_ERRNO(LOG_LEVEL_ERROR, "写%s=%s失败", key, value);
            goto ERR;
        }
    }
    temp_len = fwrite(temp_buf, 1, file_size - section_pos, _ini->fp);
    if (temp_len != (file_size - section_pos)) {
        LOG_ERRNO(LOG_LEVEL_ERROR, "fwrite失败");
        goto ERR;
    }

    mem_free(temp_buf);
    mem_free(temp_buf);

    return true;

    ERR:
    mem_free(temp_buf);
    return false;
}

bool ini_append_int(ini_t* ini, const char *section, const char *key, int value)
{
    char buf[16];
    sprintf(buf, "%d", value);
    return ini_append_str(ini, section, key, buf);
}

bool ini_append_int_array(ini_t* ini, const char *section, const char *key,
        int_array_t* values)
{
    str_array_t *str_values = str_array_new_full(32);
    int values_len = int_array_size(values);

    for (int i = 0; i < values_len; i++) {
        char* buf = mem_alloc(16);
        sprintf(buf, "%d", int_array_at(values, i));
        str_array_append(str_values, buf);
    }

    bool ret = ini_append_str_array(ini, section, key, str_values);
    str_array_free(str_values);

    return ret;
}

bool ini_get_int(ini_t* ini, const char *section, const char *key, int *value,
        int def_val)
{
    char str_value[MAX_INI_VALUE_LEN];
    char *endptr;
    bool ret;

    ret = ini_get_str(ini, section, key, str_value, sizeof(str_value), NULL);
    if (ret) {
        *value = strtol(str_value, &endptr, 10);
        if (*endptr == '\0') {
            return true;
        }
    }

    *value = def_val;

    return false;
}

bool ini_set_int(ini_t* ini, const char *section, const char *key, int value)
{
    char str_value[MAX_INI_VALUE_LEN];
    snprintf(str_value, sizeof(str_value), "%d", value);

    return ini_set_str(ini, section, key, str_value);
}

bool ini_get_int_array(ini_t* ini, const char *section, const char *key,
        int_array_t* values)
{
    str_array_t *str_values = str_array_new_full(32);
    bool ret = ini_get_str_array(ini, section, key, str_values);

    int i;

    if (!ret) { //没找到
        str_array_free(str_values);
        return false;
    }

    for (i = 0; i < str_array_size(str_values); i++) {
        const char *str_value = str_array_at(str_values, i);
        char *endptr;
        int int_value = strtol(str_value, &endptr, 10);
        if (*endptr != '\0') {
            continue;
        }
        int_array_append(values, int_value);
    }

    str_array_free(str_values);
    if (int_array_size(values) > 0) {

        return true;
    }

    return false;
}

bool ini_remove_section(ini_t* ini, const char *section)
{
    long next_section_pos;
    long section_pos;

    struct _ini *_ini = (struct _ini *) ini;
    if (_ini->fp == NULL) {
        LOG(LOG_LEVEL_ERROR, "_ini->fp is NULL");
        return false;
    }
    rewind(_ini->fp);

    section_pos = find_section_pos(ini, section);
    if (section_pos == -1) {    //没找到对应的section
        return true;
    }

    next_section_pos = find_next_section_pos(ini, section_pos);

    return ini_reaplace(ini, section_pos, next_section_pos, NULL);

}

bool ini_remove_key(ini_t* ini, const char *section, const char *key)
{
    long key_pos;
    long section_pos;
    int ret;
    char buffer[MAX_INI_LINE_LEN];

    struct _ini *_ini = (struct _ini *) ini;
    if (_ini->fp == NULL) {
        LOG(LOG_LEVEL_ERROR, "_ini->fp is NULL");
        return false;
    }
    rewind(_ini->fp);

    section_pos = find_section_pos(ini, section);
    if (section_pos == -1) {    //没找到对应的section
        return true;
    }

    while (true) {
        key_pos = find_key_pos(ini, section_pos, key);
        if (key_pos == -1) {    //没找到key
            return true;
        }

        ret = fseek(_ini->fp, key_pos, SEEK_SET);
        LOG_ERRNO_RET_FALSE_IF_ERR(ret, LOG_LEVEL_ERROR, "fseek失败");
        if (fgets(buffer, sizeof(buffer), _ini->fp) == NULL) {
            return false;
        }

        ret = ini_reaplace(ini, key_pos, ftell(_ini->fp), NULL);
        if (!ret) {
            return false;
        }
    }

    return true;
}

bool ini_get_sections(ini_t* ini, str_array_t* sections)
{
    char buffer[MAX_INI_LINE_LEN];
    char *saveptr;
    char *ptr_buffer;
    char *mark1 = NULL, *mark2 = NULL;
    int readlen;

    struct _ini *_ini = (struct _ini *) ini;

    rewind(_ini->fp);
    while (fgets(buffer, sizeof(buffer), _ini->fp) != NULL) {
        readlen = strlen(buffer);
        ptr_buffer = strtrim(buffer, readlen);
        if (ptr_buffer[0] == '[') { //段
            mark1 = (char*) strtok_r(ptr_buffer, "[", &saveptr);
            mark2 = (char*) strtok_r(mark1, "]", &saveptr);
            if (mark2 == NULL) { //段格式不正确
                continue;
            }
            mark2 = strtrim(mark2, strlen(mark2));
            str_array_append(sections, strdup2(mark2));
        }
    }

    return true;
}

bool ini_get_options(ini_t* ini, const char* section, str_array_t* keys,
        str_array_t* values)
{
    char buffer[MAX_INI_LINE_LEN];
    char *searchvalue;
    char *searchkey;
    char *saveptr;
    char *ptr_buffer;
    long section_pos;
    int readlen;
    int ret;

    struct _ini *_ini = (struct _ini *) ini;
    if (_ini->fp == NULL) {
        LOG(LOG_LEVEL_ERROR, "_ini->fp is NULL");
        return false;
    }

    rewind(_ini->fp);

    section_pos = find_section_pos(ini, section);
    if (section_pos == -1) {
        return false;
    }

    ret = fseek(_ini->fp, section_pos, SEEK_SET);
    LOG_ERRNO_RET_ERR_IF_ERR(ret, LOG_LEVEL_ERROR, "fseek失败");
    //第一行是多余的，所以先读出来
    if (fgets(buffer, sizeof(buffer), _ini->fp) == NULL) {
        return -1;
    }
    while (fgets(buffer, sizeof(buffer), _ini->fp) != NULL) {
        readlen = strlen(buffer);
        ptr_buffer = strtrim(buffer, readlen);
        if (ptr_buffer[0] == ';') {   //注释
            continue;
        } else if (ptr_buffer[0] == '[') { //段
            break;
        } else {    //key
            searchkey = (char*) strtok_r(ptr_buffer, "=", &saveptr);
            if (searchkey == NULL) {
                continue;
            }
            searchkey = strtrim(searchkey, strlen(searchkey));

            searchvalue = (char*) strtok_r(NULL, "=", &saveptr);
            if (searchvalue == NULL) {
                continue;
            }
            searchvalue = strtrim(searchvalue, strlen(searchvalue));

            str_array_append(keys, strdup2(searchkey));
            str_array_append(values, strdup2(searchvalue));
        }
    }

    if (str_array_size(keys) == 0) {
        return false;
    }

    return true;
}
