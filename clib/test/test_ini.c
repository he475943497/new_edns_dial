/*
 * test_ini.c
 *
 *  Created on: 2012-6-14
 *      Author: Administrator
 */

#include <clib/ini.h>
#include <string.h>
#include <stdio.h>
#include <clib/log.h>
#include <stdlib.h>
#include <clib/macros.h>
#include <clib/string.h>
#include <clib/log.h>
#include <clib/memory.h>
#include <clib/array.h>
#define MAX_INI_LINE_LEN 1024

void test_get_str()
{
    char *path_file = "./test/test_ini.ini";
    char value[256];
    ini_t *ini = ini_new(path_file);
    if (ini == NULL) {
        printf("open %s fail\n", path_file);
        return;
    }
    bool ret = ini_get_str(ini, "ms", "aa", value, sizeof(value), NULL);
    if (ret) {
        printf("aa = %s\n", value);
    }
    ret = ini_get_str(ini, "ms", "bb", value, sizeof(value), NULL);
    if (ret) {
        printf("bb = %s\n", value);
    }
    ret = ini_get_str(ini, "ms1", "aa1", value, sizeof(value), NULL);
    if (ret) {
        printf("aa = %s\n", value);
    }
    ret = ini_get_str(ini, "ms1", "bb1", value, sizeof(value), NULL);
    if (ret) {
        printf("bb = %s\n", value);
    }
    ini_free(ini);
}

void test_get_str_array()
{
    char *path_file = "./test/test_ini.ini";
    char *section = "ms";
    char *key = "bb";
    int i;
    //ini_t *ini = init_ini(path_file);
    ini_t *ini = ini_new(path_file);
    str_array_t *values = str_array_new_full(1);
    bool ret = ini_get_str_array(ini, section, key, values);
    if (!ret)
        printf("error!\n");
    for (i = 0; i < str_array_size(values); i++) {
        printf("ip_address = %s\n", str_array_at(values, i));
    }
    str_array_free(values);
    ini_free(ini);
}
//static void fun();

void test_ini_get_int_array()
{
    char *path_file = "/home/ini_log_test/config.ini";
    char *section = "ms";
    char *key = "conf_port";
    int i;
    ini_t *ini = ini_new(path_file);
    int_array_t *values = int_array_new();
    bool ret = ini_get_int_array(ini, section, key, values);
    if (!ret)
        printf("error!\n");
    printf("array_size(values) = %d\n", int_array_size(values));
    for (i = 0; i < int_array_size(values); i++) {
        printf("conf_port = %d\n", int_array_at(values, i));
        printf(" i = %d\n", i);
    }
    int_array_free(values);
    ini_free(ini);
}
void test_ini_get_int()
{
    char *path_file = "/home/ini_log_test/config.ini";
    char *section = "ms";
    char *key = "conf_port";
    ini_t *ini = ini_new(path_file);
    int values;
    bool ret = ini_get_int(ini, section, key, &values, 0);
    if (!ret)
        printf("error!\n");
    printf("conf_port = %d\n", values);
    ini_free(ini);
}

void test_ini_update(char* section, char* key, char* value)
{
    char buffer[MAX_INI_LINE_LEN]; //存放读取的一行文件内容
    int sleek_size;
    char *searchkey;
    bool section_found = false;
    char *ptr_buffer;
    char *mark1 = NULL, *mark2 = NULL;
    char buffer_tmp[512];
    int mem_size;
    char *saveptr;
    char *mem_p = NULL;
    char *before_mem_p = NULL;
    int flag = 0;
    int n_read;
    int before_read;
    int buf_size;
    int buf_before_size;
    int size;

    FILE *fp = fopen("/home/ini_log_test/config.txt", "r+");
    fseek(fp, 0L, SEEK_END);
    size = ftell(fp); //整个配置文件长度
    rewind(fp);
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        buf_size = strlen(buffer);
        ptr_buffer = strtrim(buffer, strlen(buffer));
        if (ptr_buffer[0] == ';') {   //注释
            continue;
        } else if (ptr_buffer[0] == '[') { //段
            mark1 = (char*) strtok_r(ptr_buffer, "[", &saveptr);
            mark2 = (char*) strtok_r(mark1, "]", &saveptr);
            if (mark2 == NULL) { //段格式不正确
                section_found = false;
                continue;
            }
            mark2 = strtrim(mark2, strlen(mark2));

            if (strncmp(mark2, section, strlen(section)) == 0) {
                section_found = true;
                continue;
            }
        } else {    //key
            if (!section_found) {    //还没有找到段，直接continue
                continue;
            }
            searchkey = (char*) strtok_r(ptr_buffer, "=", &saveptr);
            if (searchkey == NULL) {
                continue;
            }
            searchkey = strtrim(searchkey, strlen(searchkey));
            if (strncmp(searchkey, key, strlen(key)) != 0)
                continue;
            if (strncmp(searchkey, key, strlen(key)) == 0) {
                flag = 1;

                //get mod sleek
                sleek_size = ftell(fp);
                buf_before_size = sleek_size - buf_size;
                mem_size = size - sleek_size;
                //save mem_p
                mem_p = (char *) malloc(mem_size + 1);
                if (NULL == mem_p)
                    printf("error1!\n");
                n_read = fread(mem_p, 1, mem_size, fp);
                if (n_read == 0)
                    printf("error2!\n");
                rewind(fp);
                //save before_mem_p
                before_mem_p = (char *) malloc(buf_before_size + 1);
                if (NULL == before_mem_p)
                    printf("error3!\n");
                before_read = fread(before_mem_p, 1, buf_before_size, fp);
                if (before_read == 0)
                    printf("error4!\n");

                fclose(fp);
                break;
            }
        }
    }
    if (flag == 1) {
        fopen("/home/ini_log_test/config.txt", "w+");
        fwrite(before_mem_p, sizeof(char), strlen(before_mem_p), fp);
        snprintf(buffer_tmp, sizeof(buffer_tmp), "%s = %s\n", key, value);
        fwrite(buffer_tmp, sizeof(char), strlen(buffer_tmp), fp);
        fwrite(mem_p, sizeof(char), strlen(mem_p), fp);
        fclose(fp);
    }
    free(mem_p);
    free(before_mem_p);

    if (flag == 0) {    //新增的
        printf("section is not found!\n");
        return;
    }

    return;
}

bool read_ahead_section(char *str, FILE *fp)
{
    //flag = 0:之前没有的section
    //flag = 1:之前存在的section
    char *ptr_buffer;
    char buffer[MAX_INI_LINE_LEN];
    bool flag = 0;
    char *saveptr;
    char *mark1 = NULL, *mark2 = NULL;
    rewind(fp);
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {

        ptr_buffer = strtrim(buffer, strlen(buffer));
        if (ptr_buffer[0] == ';') {   //注释
            continue;
        } else if (ptr_buffer[0] == '[') { //段
            mark1 = (char*) strtok_r(ptr_buffer, "[", &saveptr);
            mark2 = (char*) strtok_r(mark1, "]", &saveptr);
            if (mark2 == NULL) { //段格式不正确
                continue;
            }
            mark2 = strtrim(mark2, strlen(mark2));

            if (strncmp(mark2, str, strlen(str)) == 0) {
                flag = 1;
                break;
            } else
                continue;
        }
    }
    return flag;
}

bool read_ahead_key(char *str, FILE *fp)
{
    //flag = 0:之前没有的key
    //flag = 1:之前存在的key
    char *ptr_buffer;
    char buffer[MAX_INI_LINE_LEN];
    bool flag = 0;
    char *searchkey;
    rewind(fp);
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {

        ptr_buffer = strtrim(buffer, strlen(buffer));
        if (ptr_buffer[0] == ';') {   //注释
            continue;
        } else if (ptr_buffer[0] == '[') { //段
            continue;
        } else if (ptr_buffer[0] != '\n' && ptr_buffer[0] != '\0') {
            searchkey = (char*) strsep(&ptr_buffer, "=");
            if (searchkey == NULL) {
                continue;
            }
            searchkey = strtrim(searchkey, strlen(searchkey));
            if (strncmp(searchkey, str, strlen(str)) != 0)
                continue;
            if (strncmp(searchkey, str, strlen(str)) == 0) {
                flag = 1;

            }
        }
    }
    return flag;
}

void test_ini_new()
{
    char buffer[MAX_INI_LINE_LEN];
    char *ptr_buffer;
    char *mark1 = NULL, *mark2 = NULL;
    char *section = "data_path";
    char *key = "testnew_three";
    char *value = "44";

    bool section_found;
    char buffer_tmp[512];
    int mem_size;
    char *saveptr_new;
    char *mem_p = NULL;
    char *before_mem_p = NULL;
    int flag = 0;
    int n_read;
    int before_read;
    int buf_size;
    int buf_before_size;
    int size;
    int sleek_size;

    FILE *fp = fopen("/home/ini_log_test/config.txt", "r+");
    rewind(fp);
    bool sct;
    sct = read_ahead_section(section, fp);
    bool ky;
    ky = read_ahead_key(key, fp);
    fclose(fp);

    //没有匹配到section，需要新增加section和key
    if (sct == 0) {
        //add new section
        //add new key and value
        printf("没有匹配到section，需要新增加section和key\n");
        FILE *fp1 = fopen("/home/ini_log_test/config.txt", "r+");
        fseek(fp1, 0L, SEEK_END);
        snprintf(buffer, sizeof(buffer), "[%s]\n", section);
        fwrite(buffer, sizeof(char), strlen(buffer), fp1);
        memset(buffer, 0, sizeof(buffer));
        snprintf(buffer, sizeof(buffer), "%s = %s\n", key, value);
        fwrite(buffer, sizeof(char), strlen(buffer), fp1);
        fclose(fp);
    }

    //匹配到section，
    if (sct == 1) {
        printf("匹配到section\n");
        //匹配到key ,根据现有的key更新value
        if (ky == 1) {
            //update key's value
            printf("匹配到key ,根据现有的key更新value\n");
            test_ini_update(section, key, value);
        }
        //匹配不到key, 在section末尾添加新的key和value
        else {
            //add new key and value
            printf("匹配不到key, 在section末尾添加新的key和value\n");
            FILE *fp = fopen("/home/ini_log_test/config.txt", "r+");
            fseek(fp, 0L, SEEK_END);
            size = ftell(fp); //整个配置文件长度
            rewind(fp);
            while (fgets(buffer, sizeof(buffer), fp) != NULL) {
                buf_size = strlen(buffer);
                ptr_buffer = strtrim(buffer, strlen(buffer));
                if (ptr_buffer[0] == ';') {   //注释
                    continue;
                } else if (ptr_buffer[0] == '[') { //段
                    mark1 = (char*) strtok_r(ptr_buffer, "[", &saveptr_new);
                    mark2 = (char*) strtok_r(mark1, "]", &saveptr_new);
                    if (mark2 == NULL) { //段格式不正确
                        section_found = false;
                        continue;
                    }
                    mark2 = strtrim(mark2, strlen(mark2));

                    if (strncmp(mark2, section, strlen(section)) == 0) {
                        flag = 1;
                        //get mod sleek
                        sleek_size = ftell(fp);
                        buf_before_size = sleek_size;
                        mem_size = size - sleek_size;
                        //save mem_p
                        mem_p = (char *) malloc(mem_size + 1);
                        if (NULL == mem_p)
                            printf("error1!\n");
                        n_read = fread(mem_p, 1, mem_size, fp);
                        if (n_read == 0)
                            printf("error2!\n");
                        rewind(fp);
                        //save before_mem_p
                        before_mem_p = (char *) malloc(buf_before_size + 1);
                        if (NULL == before_mem_p)
                            printf("error3!\n");
                        before_read = fread(before_mem_p, 1, buf_before_size,
                                fp);
                        if (before_read == 0)
                            printf("error4!\n");

                        fclose(fp);
                        break;
                    }
                }
            } //while end
            if (flag == 1) {
                printf("合并文件内容\n");
                fopen("/home/ini_log_test/config.txt", "w+");
                fwrite(before_mem_p, sizeof(char), strlen(before_mem_p), fp);
                snprintf(buffer_tmp, sizeof(buffer_tmp), "%s = %s\n", key,
                        value);
                fwrite(buffer_tmp, sizeof(char), strlen(buffer_tmp), fp);
                fwrite(mem_p, sizeof(char), strlen(mem_p), fp);
                fclose(fp);
            }
            free(mem_p);
            free(before_mem_p);

        } //else
    } //if
    return;
}
void test_ini_append_str()
{
    char *path_file = "./test/test_ini.ini";

    ini_t *ini = ini_new(path_file);
    if (ini == NULL) {
        printf("open %s fail\n", path_file);
        return;
    }
    bool ret = ini_append_str(ini, "ms", "bb", "dddd1111");
    if (!ret) {
        printf("ini_append_str fail\n");

    }
    ini_free(ini);
}

void test_ini_append_str_array(ini_t *ini)
{
    str_array_t *values = str_array_new_full(1);
    str_array_append(values, strdup2("ccc44"));
    str_array_append(values, strdup2("ccc55"));
    bool ret = ini_append_str_array(ini, "172", "ccc", values);
    ret = ini_append_str_array(ini, "17", "ccc", values);
    str_array_free(values);
    if (!ret) {
        printf("ini_append_str_array fail\n");

    }
}

void test_ini_set_str()
{
    char *path_file = "./test/test_ini.ini";

    ini_t *ini = ini_new(path_file);
    if (ini == NULL) {
        printf("open %s fail\n", path_file);
        return;
    }
    bool ret = ini_set_str(ini, "ms", "bb", "2");
    if (!ret) {
        printf("ini_append_str fail\n");

    }
    ini_free(ini);
}

void test_ini_remove_key()
{
    char *path_file = "./test/test_ini.ini";

    ini_t *ini = ini_new(path_file);
    if (ini == NULL) {
        printf("open %s fail\n", path_file);
        return;
    }
    bool ret = ini_remove_key(ini, "ms1", "bb");
    if (!ret) {
        printf("ini_append_str fail\n");

    }
    ini_free(ini);
}

void test_ini_remove_section()
{
    char *path_file = "./test/test_ini.ini";

    ini_t *ini = ini_new(path_file);
    if (ini == NULL) {
        printf("open %s fail\n", path_file);
        return;
    }
    bool ret = ini_remove_section(ini, "ms1");
    if (!ret) {
        printf("ini_append_str fail\n");

    }
    ini_free(ini);
}

void test_ini_get_sections(ini_t *ini)
{
    str_array_t *sections = str_array_new_full(32);
    ini_get_sections(ini, sections);
    for (int i = 0; i < str_array_size(sections); i++) {
        printf("section = %s\n", str_array_at(sections, i));
    }
    str_array_free(sections);
}

void test_ini_get_options(ini_t *ini)
{
    str_array_t *keys = str_array_new_full(32);
    str_array_t *values = str_array_new_full(32);
    ini_get_options(ini, "ms1", keys, values);
    for (int i = 0; i < str_array_size(keys); i++) {
        printf("%s = %s\n", str_array_at(keys, i),str_array_at(values, i));
    }
    str_array_free(keys);
    str_array_free(values);
}

void test_ini()
{
    char *path_file = "./test/test_ini.ini";

    ini_t *ini = ini_new(path_file);
    if (ini == NULL) {
        printf("open %s fail\n", path_file);
        return;
    }

//    test_ini_get_sections( ini);
//    test_ini_get_int_array();
//    test_get_str();
    //test_ini_update();
//    test_ini_remove_key();
//    test_ini_append_str();
    test_ini_append_str_array(ini);
//    test_get_str_array();
    //test_ini_get_int();
//    test_ini_new();
//    test_ini_get_options(ini);
    ini_free(ini);
}
