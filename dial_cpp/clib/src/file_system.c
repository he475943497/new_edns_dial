/*
 * file_system.c
 *
 *  Created on: 2012-8-13
 *      Author: Administrator
 */
#include <clib/file_system.h>
#include <dirent.h>
#include <sys/stat.h>
#include <clib/log.h>
#include <clib/memory.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <clib/topn.h>
 #include <sys/vfs.h>    /* or <sys/statfs.h> */

struct subdir {
    char d_name[NAME_MAX];
    struct stat st;
};

int subdirs(const char* dir, str_array_t *sub_dirs, dir_filter filter)
{
    struct dirent* ent = NULL;
    DIR *pDir;
    char *path_name;
    int path_name_len;
    struct stat st;
    char tempbuf[512];

    pDir = opendir(dir);
    if (pDir == NULL) {
        LOG_ERRNO(LOG_LEVEL_ERROR, dir);
        return -1;
    }
    while (NULL != (ent = readdir(pDir))) {
        //printf("d_reclen:%d\td_type:%d\td_name:%s\n", ent->d_reclen,
        //      ent->d_type, ent->d_name);
        if (dir[strlen(dir) - 1] == '/') {
            snprintf(tempbuf, sizeof(tempbuf), "%s%s", dir, ent->d_name);
        } else {
            snprintf(tempbuf, sizeof(tempbuf), "%s/%s", dir, ent->d_name);
        }
        bzero(&st, sizeof(struct stat));
        lstat(tempbuf, &st);
        if (S_ISDIR(st.st_mode)) {
            if (filter != NULL && !filter(dir, ent->d_name)) {
                continue;
            }
            path_name_len = strlen(ent->d_name) + 1;
            path_name = (char*) mem_alloc(sizeof(char) * path_name_len);
            strcpy(path_name, ent->d_name);
            str_array_append(sub_dirs, path_name);
        }
    }

    closedir(pDir);

    return 0;
}

int subfiles(const char* dir, str_array_t *files, file_filter filter)
{
    struct dirent* ent = NULL;
    DIR *pDir;
    char *path_name;
    int path_name_len;
    struct stat st;
    char tempbuf[512];

    pDir = opendir(dir);
    if (pDir == NULL) {
        LOG_ERRNO(LOG_LEVEL_ERROR, dir);
        return -1;
    }
    while (NULL != (ent = readdir(pDir))) {
        //printf("d_reclen:%d\td_type:%d\td_name:%s\n", ent->d_reclen,
        //      ent->d_type, ent->d_name);
        if (dir[strlen(dir) - 1] == '/') {
            snprintf(tempbuf, sizeof(tempbuf), "%s%s", dir, ent->d_name);
        } else {
            snprintf(tempbuf, sizeof(tempbuf), "%s/%s", dir, ent->d_name);
        }
        bzero(&st, sizeof(struct stat));
        lstat(tempbuf, &st);
        if (S_ISREG(st.st_mode)) {
            if (filter != NULL && !filter(dir, ent->d_name)) {
                continue;
            }
            path_name_len = strlen(ent->d_name) + 1;
            path_name = (char*) mem_alloc(sizeof(char) * path_name_len);
            strcpy(path_name, ent->d_name);
            str_array_append(files, path_name);
        }
    }

    closedir(pDir);

    return 0;
}

static bool append_subdir(void *key, void *value, void *user_data)
{
    str_array_t *sub_dirs = (str_array_t *) user_data;
    str_array_append(sub_dirs, key);
    return false;
}

int subfiles_order_by_name(const char* dir, str_array_t *files,
        file_filter filter)
{
    str_array_t *all_files = str_array_new();

    int ret = subfiles(dir, all_files, filter);
    if (ret == -1) {
        return -1;
    }

    topn_t *topn_file = topn_new(str_array_size(all_files), str_compare_func,
            NULL, NULL, NULL);
    for (size_t i = 0; i < str_array_size(all_files); i++) {
        topn_insert(topn_file, str_array_at(all_files, i), NULL);
    }
    str_array_free(all_files);

    topn_foreach(topn_file, append_subdir, files);

    topn_free(topn_file);

    return 0;
}

int subdirs_order_by_name(const char* dir, str_array_t *sub_dirs,
        dir_filter filter)
{
    str_array_t *all_dirs = str_array_new();

    int ret = subdirs(dir, all_dirs, filter);
    if (ret == -1) {
        return -1;
    }

    topn_t *topn_dir = topn_new(str_array_size(all_dirs), str_compare_func,
            NULL, NULL, NULL);
    for (int i = 0; i < str_array_size(all_dirs); i++) {
        topn_insert(topn_dir, str_array_at(all_dirs, i), NULL);
    }
    str_array_free(all_dirs);

    topn_foreach(topn_dir, append_subdir, sub_dirs);

    topn_free(topn_dir);

    return 0;
}



int free_space(const char* dir)
{
    struct statfs buf;
    int b = statfs(dir, &buf);

    if (!b) {
//        printf("free space: %lu M", (buf.f_bfree * 4)/1024);
        return (buf.f_bavail * 4)/1024;
    }
    LOG_ERRNO(LOG_LEVEL_WARNING, "获取目录%s剩余空间失败", dir);
    return -1;
}
