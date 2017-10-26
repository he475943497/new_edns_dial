/*
 * memory.h
 *
 *  Created on: 2012-6-7
 *      Author: Administrator
 */

#ifndef CLIB_MEMORY_H_
#define CLIB_MEMORY_H_

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CHECK_MEM_LEAK
    #include <clib/memleak.h>
    #define mem_alloc(x) mem_leak_alloc(x, __FILE__, __LINE__)
    #define mem_free(m) mem_leak_free(m)
#else
    #define mem_alloc(x) malloc(x)
    #define mem_free(m) free(m)
#endif


#ifdef __cplusplus
}
#endif

#endif /* CLIB_MEMORY_H_ */
