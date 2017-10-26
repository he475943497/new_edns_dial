/**
 *Author:           wuyangchun
 *Date:             2012-06-01
 *
 *Description:      常见的字符串处理函数
 *
 *Notice:           多线程安全
**/
#ifndef CLIB_STRING_H_
#define CLIB_STRING_H_

#include <clib/types.h>
#include <clib/array.h>
#include <string.h>


#ifdef __cplusplus
extern "C" {
#endif

/**
 * streq - Are two strings equal?
 * @a: first string
 * @b: first string
 *
 * This macro is arguably more readable than "!strcmp(a, b)".
 *
 * Example:
 *	if (streq(somestring, ""))
 *		printf("String is empty!\n");
 */
#define streq(a,b) (strcmp((a), (b)) == 0)

/**
 * strstarts - Does this string start with this prefix?
 * @str: string to test
 * @prefix: prefix to look for at start of str
 *
 * Example:
 *	if (strstarts(somestring, "foo"))
 *		printf("String %s begins with 'foo'!\n", somestring);
 */
#define strstarts(str,prefix) (strncmp((str), (prefix), strlen(prefix)) == 0)

/**
 * strends - Does this string end with this postfix?
 * @str: string to test
 * @postfix: postfix to look for at end of str
 *
 * Example:
 *	if (strends(somestring, "foo"))
 *		printf("String %s end with 'foo'!\n", somestring);
 */
static inline bool strends(const char *str, const char *postfix)
{
    if (strlen(str) < strlen(postfix))
        return false;

    return streq(str + strlen(str) - strlen(postfix), postfix);
}

/**
 * stringify - Turn expression into a string literal
 * @expr: any C expression
 *
 * Example:
 *	#define PRINT_COND_IF_FALSE(cond) \
 *		((cond) || printf("%s is false!", stringify(cond)))
 */
#define stringify(expr) #expr


/**
 * strcount - Count number of (non-overlapping) occurrences of a substring.
 * @haystack: a C string
 * @needle: a substring
 *
 * Example:
 *      assert(strcount("aaa aaa", "a") == 6);
 *      assert(strcount("aaa aaa", "ab") == 0);
 *      assert(strcount("aaa aaa", "aa") == 2);
 */
size_t strcount(const char *haystack, const char *needle);


/*
 *@brief        去掉字符串前面和后面的空格以及回车
 *@param in     str     待处理的字符串
 *@param out    strlen  待处理的字符串长度
 *@return       处理结果字符串
 *
 *@notice       该函数会修改传入的str的值，并且返回str的一个子串
 *              例如：char str[] = " hello  ";
 *              char *result = strtrim(str, strlen(str));
 *              结果：      str 变成 " hello"
 *                      result 指向地址 &(str[1])，值为"hello"
 */
char * strtrim(char *str, int strlen);

/*
 *@brief        内存拷贝
 *@param in     mem     待拷贝的源地址
 *@param out    size    待拷贝的内存长度
 *@return       新的内存空间
 *
 *@notice       返回的内存空间长度等于传入的参数size，内容等于mem
 *              空间分配在堆上，需要调用者手动释放
 *
 */
void* memdup(const void *mem, uint size);

/*
 *@brief        字符长拷贝
 *@param in     str     待拷贝的源字符串
 *@return       新的字符串
 *
 *@notice       返回的字符串需要调用者手动释放
 *              string.h中已经包含了同样功能的strdup函数
 *              如果调用strdup的话，memleak就检测不到他是否释放
 *              strdup2替代strdup主要是为了做内存泄露检查
 */
char* strdup2(const char *str);

/*
 *@brief        分割字符串
 *@param in     str     待处理的字符串
 *@return       得到的字符串数组
 *
 *@notice       返回的array需要调用者自己释放
 */
str_array_t* strsplit(const char *str, const char *sep);

/*
 *@brief        将字符串转换成小写
 *@param in     str     待转换的字符串
 *@return
 *
 *@notice
 */
void str_tolower(char *str);

/*
 *@brief        查找字符串中最后一个指定的字符
 *@param in     str     待操作的字符串
 *@param in     ch     待查找的字符
 *@return       找到的字符位置，如果没找到，返回NULL
 *
 *@notice
 */
char* strlast(char* str, char ch);

/*
 *@brief        判断字符串是否是以指定字符串结尾
 *@param in     str     待操作的字符串
 *@param in     end     待查找的字符串
 *@return       如果str是以end结尾，返回true，否则返回false
 *
 *@notice
 */
bool str_endwith(const char* str, const char* end);


#ifdef __cplusplus
}
#endif

#endif /* CLIB_STRING_H_ */
