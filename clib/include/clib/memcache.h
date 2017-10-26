#ifndef MEMCACHE_H
#define MEMCACHE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mem_cache mem_cache_t;

extern mem_cache_t *mem_cache_create(int obj_size, int cache_size);
extern int mem_cache_destroy(mem_cache_t *p);


extern void *mem_cache_alloc(mem_cache_t *p);
extern int mem_cache_free(mem_cache_t *p, void *obj);

extern int mem_cache_dump_all();
extern int mem_cache_dump(mem_cache_t *p);

#ifdef __cplusplus
}
#endif
#endif 
