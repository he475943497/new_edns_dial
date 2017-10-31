#ifndef MEMLEAK_H
#define MEMLEAK_H

#ifdef __cplusplus
extern "C" {
#endif

extern void *mem_leak_alloc(unsigned int size, char *file, int line);
extern int mem_leak_free(void *mem);
extern void mem_leak_dump();


#ifdef __cplusplus
}
#endif

#endif
