#include "clib/log.h"
#include "clib/memcache.h"
#include "clib/memory.h"

#include <stdio.h>
#include <pthread.h>

struct mem_obj{
    int size;
    int inuse;
    void * end ;
    struct mem_obj *next;
    unsigned int obj_free;
    unsigned int *obj_ctl;
    void *obj;
};

struct mem_cache{
    int obj_size;
    int cache_size;
    unsigned int reciprocal;
    struct mem_obj *list;
    struct mem_obj *wait_for_free;
    struct mem_cache *next;
};

static pthread_mutex_t l_memcache_lock = PTHREAD_MUTEX_INITIALIZER;
static mem_cache_t *head = NULL;

#ifdef __i386__
#define do_div(n, base)                                         \
    ({                                                          \
        unsigned long __upper, __low, __high, __mod, __base;    \
        __base = (base);                                        \
        asm("":"=a" (__low), "=d" (__high) : "A" (n));          \
        __upper = __high;                                       \
                                                                \
        if (__high) {                                           \
            __upper = __high % (__base);                        \
            __high = __high / (__base);                         \
        }                                                       \
        asm("divl %2":"=a" (__low), "=d" (__mod)                \
            : "rm" (__base), "0" (__low), "1" (__upper));       \
        asm("":"=A" (n) : "a" (__low), "d" (__high));           \
        __mod;                                                  \
    })

static inline unsigned int reciprocal_divide(unsigned int A, unsigned int R)
{
    return (unsigned int) (((unsigned long long) A * R) >> 32);
}

#elif __x86_64__
#define do_div(n,base) ({                               \
            unsigned int __base = (base);               \
            unsigned int __rem;                         \
            __rem = ((unsigned long long)(n)) % __base; \
            (n) = ((unsigned long long)(n)) / __base;   \
            __rem;                                      \
        })

static inline unsigned int reciprocal_divide(unsigned long long A, unsigned int R)
{
    return (unsigned int)(((unsigned long long)A * R) >> 32);
}

#endif 

unsigned int reciprocal_value(unsigned int k)
{
    unsigned long long val = (1LL << 32) + (k - 1);
    do_div(val, k);
    return (unsigned int) val;
}

static inline unsigned int obj_to_index(mem_cache_t *pc,
        struct mem_obj *po, void *obj)
{
    return reciprocal_divide(obj - po->obj, pc->reciprocal);
}

static inline void *index_to_obj(mem_cache_t *pc, struct mem_obj *po,
        unsigned int idx)
{
    return po->obj + idx * pc->obj_size;
}

mem_cache_t *mem_cache_create(int obj_size, int cache_size)
{
    mem_cache_t *p;
    p = (mem_cache_t *) mem_alloc(sizeof(mem_cache_t)*sizeof(char));
    if (p == NULL) {
        LOG(LOG_LEVEL_ERROR, "%s", "malloc error!!");
        return NULL;
    }
    p->obj_size = obj_size;
    p->cache_size = cache_size;
    p->reciprocal = reciprocal_value(p->obj_size);
    p->list = NULL;
    p->wait_for_free = NULL;
    pthread_mutex_lock(&l_memcache_lock);
    p->next = head;
    head = p;
    pthread_mutex_unlock(&l_memcache_lock);
    return p;
}

int mem_cache_destroy(mem_cache_t *p)
{
    mem_cache_t *pr = NULL, *pc = head;
    struct mem_obj *pl, *po;
    pthread_mutex_lock(&l_memcache_lock);
    while (pc) {
        if (pc == p) {
            if (pr == NULL)
                head = pc->next;
            else
                pr->next = pc->next;
            break;
        }
        pr = pc;
        pc = pc->next;
    }

    if (pc != NULL) {
        po = pc->list;
        while (po) {
            if (po->obj_ctl)
                mem_free(po->obj_ctl);
            pl = po;
            po = po->next;
            mem_free(pl);
        }
        po = pc->wait_for_free;
        if (po){
            if(po->obj_ctl)
                mem_free(po->obj_ctl);
            mem_free(po);
        }
        mem_free(pc);
    }
    pthread_mutex_unlock(&l_memcache_lock);
    return 0;
}

static int cache_grow(mem_cache_t *p)
{
    struct mem_obj *po;
    if (p->wait_for_free != NULL) {
        po = p->wait_for_free;
        p->wait_for_free = NULL;
        goto direct_add;
    }
    po = (struct mem_obj *) mem_alloc(sizeof(struct mem_obj)*sizeof(char));
    if (po == NULL) {
        LOG(LOG_LEVEL_ERROR, "%s", "malloc error!!");
        return -1;
    }
    po->obj_ctl = mem_alloc((p->obj_size+sizeof(po->obj_free)) * p->cache_size);
    if (po->obj_ctl == NULL) {
        LOG(LOG_LEVEL_ERROR, "%s", "malloc error!!");
        return -1;
    }
    po->obj = (void *) po->obj_ctl + sizeof(po->obj_free) * p->cache_size;
    for (int i = 0; i < p->cache_size; i++) {
        po->obj_ctl[i] = i + 1;
    }
    po->size = p->cache_size;
    po->inuse = 0;
    po->obj_free = 0;
    po->end = (void *) po->obj_ctl
            + (p->obj_size + sizeof(po->obj_free)) * p->cache_size;

    direct_add: po->next = p->list;
    p->list = po;
    return 0;
}

void *mem_cache_alloc(mem_cache_t *p)
{
    struct mem_obj *tmp;
    void *p_obj = NULL;
    re_alloc: tmp = p->list;
    while (tmp) {
        if (tmp->inuse < tmp->size) {
            p_obj = index_to_obj(p, tmp, tmp->obj_free);
            tmp->obj_free = tmp->obj_ctl[tmp->obj_free];
            tmp->inuse++;
            return p_obj;
        }
        tmp = tmp->next;
    }
    if (cache_grow(p) == -1)
        return NULL;
    goto re_alloc;
}

static int mem_obj_free(mem_cache_t *p, struct mem_obj *po)
{
    struct mem_obj *tmp;
    if (p->wait_for_free != NULL) {
        tmp = p->wait_for_free;
        mem_free(tmp->obj_ctl);
        mem_free(tmp);
    }
    tmp = p->list;
    if (tmp == po) {
        p->list = po->next;
        p->wait_for_free = po;
        return 0;
    }
    while (tmp) {
        if (tmp->next == po) {
            tmp->next = po->next;
            p->wait_for_free = po;
            return 0;
        }
        tmp = tmp->next;
    }
    return -1;
}

int mem_cache_free(mem_cache_t *p, void *obj)
{
    struct mem_obj *po;
    unsigned int idx;
    po = p->list;
    while (po) {
        if ((void *) po->obj_ctl < obj && obj < po->end)
            break;
        po = po->next;
    }
    if (po == NULL) {
        LOG(LOG_LEVEL_ERROR, "%s", "free error!!");
        return -1;
    }
    idx = obj_to_index(p, po, obj);
    po->obj_ctl[idx] = po->obj_free;
    po->obj_free = idx;
    po->inuse--;
    if (po->inuse == 0)
        mem_obj_free(p, po);
    return 0;
}

int mem_cache_dump_all()
{
    int i = 1;
    mem_cache_t *pc = head;
    struct mem_obj *po;
    pthread_mutex_lock(&l_memcache_lock);
    while (pc) {
        po = pc->list;
        LOG(LOG_LEVEL_WARNING, "%s%d%s%d",
                "mem cache:", i, "obj size:", pc->obj_size);
        while (po) {
            LOG(LOG_LEVEL_WARNING, "%s%d%s%d%s%d%s%x%s%x%s%x",
                    "size:", po->size, " inuse:", po->inuse, " free:", po->obj_free, " ctl start:", po->obj_ctl, " obj start:", po->obj, " end:", po->end);
            po = po->next;
        }
        if (pc->wait_for_free) {
            LOG(LOG_LEVEL_WARNING, "%s", "have wait for free");
        } else {
            LOG(LOG_LEVEL_WARNING, "%s", "none wait for free");
        }
        pc = pc->next;
        i++;
    }
    pthread_mutex_unlock(&l_memcache_lock);
    return 0;
}

int mem_cache_dump(mem_cache_t *p)
{
    struct mem_obj *po;
    po = p->list;
    LOG(LOG_LEVEL_WARNING, "%s%x%s%d",
            "mem cache:", p, "obj size:", p->obj_size);
    while (po) {
        LOG(LOG_LEVEL_WARNING, "%s%d%s%d%s%d%s%x%s%x%s%x",
                "size:", po->size, " inuse:", po->inuse, " free:", po->obj_free, " ctl start:", po->obj_ctl, " obj start:", po->obj, " end:", po->end);
        po = po->next;
    }
    if (p->wait_for_free) {
        LOG(LOG_LEVEL_WARNING, "%s", "have wait for free");
    } else {
        LOG(LOG_LEVEL_WARNING, "%s", "none wait for free");
    }
    return 0;
}
