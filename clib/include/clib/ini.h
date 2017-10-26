/**
 *Author:           wuyangchun,guozongchuan
 *Date:             2012-06-01
 *Description:      配置文件读写, section不能重复，section下的key可以重复
 *
 *Notice:           考虑到对同一个配置文件的操作一般不会分布在多个线程中
 *                  所以对同一个配置文件的操作，不保证多线程安全
 *                  其他情况是多线程安全的
 **/

#ifndef CLIB_INI_H_
#define CLIB_INI_H_

#include <clib/array.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_INI_KEY_LEN 64
#define MAX_INI_VALUE_LEN 256

typedef struct _ini ini_t;

/*
 *@brief        创建读写ini配置的对象
 *@param in     ini_file    配置文件名称
 *@return       NULL表示失败
 */
ini_t* ini_new(const char *ini_file);

/*
 *@brief        释放读写ini配置的对象
 *@param in     ini    ini对象
 *@return       NULL表示失败
 */
void ini_free(ini_t* ini);

/*
 *@brief        获取字符串格式的值
 *@param in     ini         ini对象
 *@param in     section     段名称
 *@param in     key         key名称
 *@param out    value       key对应的值
 *@param in     value_size   value的长度
 *@param in     def_val     当根据key没找到对应的配置项时，value被赋值为def_val
 *@return       true/false  成功/失败
 *
 *@notice       返回的字符串已经经过strtrim处理，去掉了开始和结束处的空格回车
 */
bool ini_get_str(ini_t* ini, const char *section, const char *key,
        char *value, uint value_size, const char *def_val);

/*
 *@brief        设置字符串格式的值（找到第一个key，并将其值改成指定的值，如果key不存在，添加相应的key）
 *@param in     ini         ini对象
 *@param in     section     段名称
 *@param in     key         key名称
 *@param in     value       key对应的值
 *@return       true/false  成功/失败
 */
bool ini_set_str(ini_t* ini, const char *section, const char *key,
        const char *value);

/*
 *@brief        添加字符串格式的值
 *@param in     ini         ini对象
 *@param in     section     段名称
 *@param in     key         key名称
 *@param out    value       key对应的值
 *@return       true/false  成功/失败
 */
bool ini_append_str(ini_t* ini, const char *section, const char *key, const char *value);

/*
 *@brief        添加字符串格式的值
 *@param in     ini         ini对象
 *@param in     section     段名称
 *@param in     key         key名称
 *@param out    value       key对应的值
 *@return       true/false  成功/失败
 */
bool ini_append_str_array(ini_t* ini, const char *section, const char *key, str_array_t* values);


/*
 *@brief        获取字符串格式的值，返回值的顺序和配置文件中的先后顺序一样
 *@param in     ini         ini对象
 *@param in     section     段名称
 *@param in     key         key名称
 *@param out    values      key对应的值数组（多个）
 *@return       true/false  成功/失败
 *
 *@notice       返回的字符串已经经过strtrim处理，去掉了开始和结束处的空格回车
 */
bool ini_get_str_array(ini_t* ini, const char *section, const char *key,
        str_array_t* values);

/*
 *@brief        获取int格式的值
 *@param in     ini         ini对象
 *@param in     section     段名称
 *@param in     key         key名称
 *@param out    value       key对应的值
 *@param in     def_val     当根据key没找到对应的配置项时，value被赋值为def_val
 *@return       true/false  成功/失败
 */
bool ini_get_int(ini_t* ini, const char *section, const char *key, int *value,
        int def_val);

/*
 *@brief        设置int格式的值（找到第一个key，并将其值改成指定的值，如果key不存在，添加相应的key）
 *@param in     ini         ini对象
 *@param in     section     段名称
 *@param in     key         key名称
 *@param in     value       key对应的值
 *@return       true/false  成功/失败
 */
bool ini_set_int(ini_t* ini, const char *section, const char *key, int value);

/*
 *@brief        添加int格式的值
 *@param in     ini         ini对象
 *@param in     section     段名称
 *@param in     key         key名称
 *@param out    value       key对应的值
 *@return       true/false  成功/失败
 */
bool ini_append_int(ini_t* ini, const char *section, const char *key, int value);

/*
 *@brief        添加int格式的值
 *@param in     ini         ini对象
 *@param in     section     段名称
 *@param in     key         key名称
 *@param out    value       key对应的值
 *@return       true/false  成功/失败
 */
bool ini_append_int_array(ini_t* ini, const char *section, const char *key, int_array_t* values);

/*
 *@brief        获取数字格式的值，返回值的顺序和配置文件中的先后顺序一样
 *@param in     ini         ini对象
 *@param in     section     段名称
 *@param in     key         key名称
 *@param out    values      key对应的值数组（多个）
 *@return       true/false  成功/失败
 */
bool ini_get_int_array(ini_t* ini, const char *section, const char *key,
        int_array_t* values);

/*
 *@brief        获取ini文件中的所有section
 *@param in     ini         ini对象
 *@param out    sections    取到的所有section
 *@return       true/false  成功/失败
 */
bool ini_get_sections(ini_t* ini, str_array_t* sections);

/*
 *@brief        获取ini文件中指定section下所有的配置项
 *@param in     ini         ini对象
 *@param in     ini         需要获取的section
 *@param out    keys        取到的所有key
 *@param out    values      取到的所有value
 *@return       true/false  成功/失败
 */
bool ini_get_options(ini_t* ini, const char* section, str_array_t* keys, str_array_t* values);

/*
 *@brief        删除ini文件中的整个section
 *@param in     ini         ini对象
 *@param in     section     段名称
 *@return       true/false  成功/失败
 */
bool ini_remove_section(ini_t* ini, const char *section);

/*
 *@brief        删除ini文件中的section下所有key名称等于指定值的项
 *@param in     ini         ini对象
 *@param in     section     段名称
 *@param in     key         key名称
 *@return       true/false  成功/失败
 */
bool ini_remove_key(ini_t* ini, const char *section, const char *key);

#ifdef __cplusplus
}
#endif
#endif /* CLIB_INI_H_ */
