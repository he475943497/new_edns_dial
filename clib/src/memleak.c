#include <limits.h>
#include <stdio.h>
#include "clib/log.h"
#include "clib/memory.h"
#include <pthread.h>

static pthread_mutex_t l_memleak_lock = PTHREAD_MUTEX_INITIALIZER;

struct mem_leak{
    unsigned int size;
    void *mem;
    char file[NAME_MAX];
    int line;
    struct mem_leak *next;
};

struct mem_leak *mem_head = NULL;
static unsigned int mem_total = 0; 

void *mem_leak_alloc(unsigned int size, char *file, int line)
{
    void *ptr;
    ptr = malloc(size);
    if(ptr == NULL){
	    LOG(LOG_LEVEL_ERROR, "%s", "malloc error!!");
        return ptr;
	}
    struct mem_leak *pml = malloc(sizeof(struct mem_leak));
    if(pml == NULL){
        free(ptr);
        LOG(LOG_LEVEL_ERROR, "%s", "malloc error!!");
        return NULL;
    }
    pml->mem = ptr;
    pml->size = size;
    snprintf(pml->file, NAME_MAX, "%s", file);
    pml->line = line; 
    pthread_mutex_lock(&l_memleak_lock);
    pml->next = mem_head;
    mem_head = pml;
    mem_total += size;
    pthread_mutex_unlock(&l_memleak_lock);
    return ptr;
}

int mem_leak_free(void *mem)
{
    struct mem_leak *ptr, *pre;
    pthread_mutex_lock(&l_memleak_lock);
    ptr = mem_head;
    while(ptr){
        if(ptr->mem == mem){
            if(ptr == mem_head)
                mem_head = ptr->next;
            else
                pre->next = ptr->next;
            mem_total -= ptr->size;
            free(ptr);
            free(mem);
            pthread_mutex_unlock(&l_memleak_lock);
            return 0;
        }
        pre = ptr;
        ptr = ptr->next;
    }
    pthread_mutex_unlock(&l_memleak_lock);
//    LOG(LOG_LEVEL_ERROR, "free error:%x", mem);

    return 0;   
}

void mem_leak_dump()
{
    struct mem_leak *ptr;
    unsigned int total_leak_size = 0;
    LOG(LOG_LEVEL_WARNING, "%s", "memory leak dump");
    ptr = mem_head;
    while(ptr){
      LOG(LOG_LEVEL_WARNING, "%s%d%s%x%s%s%s%d", 
      "!!memory  leak!! size:", ptr->size, 
      " Addr", ptr->mem, 
      " File:", ptr->file,
      " Line:", ptr->line);
      total_leak_size += ptr->size;
      ptr = ptr->next;
      }
    LOG(LOG_LEVEL_WARNING, "%s%d", "memory leak total:", mem_total);
    return ;
}
