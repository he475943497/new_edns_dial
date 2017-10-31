#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <clib/ip_hash.h>
#include <clib/memory.h>

#define MAPPING_256_TO_1       0
#define MAPPING_256_TO_256     1



typedef struct mapping_table{
    int type;
    void *mapping;
    int ref_count;
    struct mapping_table *next;
}mapping_table;

typedef struct ip_hash_priv_t{
    void *hash;
    struct mapping_table *mapping_list_h;
    list_t *list_pool;
}ip_hash_priv_t;

static list_t *malloc_list(ip_hash_priv_t *hash)
{
    list_t * lt;
    lt = list_new(4, NULL);
    list_append(hash->list_pool, lt);
    return lt;
}

static void free_list(ip_hash_priv_t *hash, list_t *lt)
{
    return ;
}

static mapping_table *malloc_mapping_table(ip_hash_priv_t *hash, int num)
{
    mapping_table *pmt;
    if(hash == NULL || num == 0)
        return NULL;
    pmt = (mapping_table *)mem_alloc(sizeof(mapping_table));
    if(pmt == NULL){
        return NULL;
    }
    pmt->mapping = mem_alloc(sizeof(list_t*)*num);
    if(pmt->mapping == NULL){
        mem_free(pmt);
        return NULL;
    }
    pmt->ref_count = 0;
    list_t **tmp = pmt->mapping;
    for(int i = 0; i < num; i++){
        tmp[i] = NULL;
    }
    pmt->next = hash->mapping_list_h;
    hash->mapping_list_h = pmt;
    return pmt;
}

static int free_mapping_table(ip_hash_priv_t *hash, mapping_table *pmt)
{
    mapping_table **ppmt;
    if(--pmt->ref_count > 0)
        return 0;
    ppmt = &(hash->mapping_list_h);
    while(*ppmt){
        if(*ppmt == pmt){
            *ppmt = pmt->next;
            break;
        }         
        ppmt = &((*ppmt)->next);
    }
    if(pmt->mapping != NULL)
        mem_free(pmt->mapping);
    mem_free(pmt);
    return 0;
}

ip_hash_t * ip_hash_init()
{
    mapping_table **hash;
    mapping_table **ppmt;
    struct ip_hash_priv_t *priv;
    priv = mem_alloc(sizeof(ip_hash_priv_t));
    if(priv == NULL){
        return NULL;
    }
    hash = mem_alloc(sizeof(mapping_table*)*(1<<24));
    if(hash == NULL){
        mem_free(priv);
        return NULL;
    }
    //printf("hash addr:%p, size:%lu\n", hash, (sizeof(mapping_table*)*policy_num)*(1<<24));
    ppmt = hash;
    unsigned long i=0;
    for(i = 0; i < (1<<24); i++){
        ppmt[i] = NULL;
    }
    priv->hash = hash;
    priv->mapping_list_h = NULL;
    priv->list_pool = list_new(16, list_free);
    return (ip_hash_t *)priv;
}

void ip_hash_uninit(ip_hash_t *ip_hash)
{
    ip_hash_priv_t *priv = (ip_hash_priv_t *)ip_hash;
    mapping_table *pmt, *pmt_prev;
    if(priv->hash != NULL)
        mem_free(priv->hash);
    if(priv->mapping_list_h != NULL){
        pmt = priv->mapping_list_h;
        while(pmt){
            if(pmt->mapping != NULL)
                mem_free(pmt->mapping);
            pmt_prev = pmt;
            pmt = pmt->next;
            mem_free(pmt_prev);
        }
    }
    if(priv->list_pool != NULL){
        list_free(priv->list_pool);
    }
    mem_free(priv);
}

#define HASH_KEY(ip)    (ip>>8)

#define HASH_KEY_2(ip)  (ip&0x000000FF)

#define HASH_ADDR(h)    (h->hash)

#define HASH_KEY_TO_MP(hash, key)   (HASH_ADDR(hash) + key*sizeof(mapping_table *))

bool print(void *data, void *user_data)
{
    printf("value :%d\n", *(int *)data);
    return false;
}
 
static int cmp_data(const void *a, const void *b, void *user_data)
{
    if(a == b)
        return 0;
    else
        return 1;
}


int ip_hash_add(ip_hash_t *ip_hash, uint start, uint end, void *ptr)
{
    void **pptr;
    void *tmp;
    mapping_table *pmt;
    unsigned int h,t,s,e;
    
    if(ip_hash == NULL || start > end || ptr == NULL)
        return -1;

    h = HASH_KEY_2(start);
    t = HASH_KEY_2(end);
    s = HASH_KEY(start);
    e = HASH_KEY(end);

    mapping_table **node = HASH_KEY_TO_MP(ip_hash, s);

    if(e == s && (h != 0 || t != 255)){
        if(*node == NULL || (*node)->type == MAPPING_256_TO_1){
            pmt = malloc_mapping_table((ip_hash_priv_t *)ip_hash, 256);
            if(pmt == NULL){
                return -1;
            }
            pmt->type = MAPPING_256_TO_256;
            pptr = pmt->mapping;
            if(*node == NULL){
                list_t *lt = malloc_list((ip_hash_priv_t *)ip_hash);
                list_append(lt, ptr);
                for(unsigned int i = h; i <= t; i++){                   
                    pptr[i] = lt;
                }
            }else{
                pptr = (*node)->mapping;
                tmp = pptr[0];
                pptr = pmt->mapping;
                list_t *lt = malloc_list((ip_hash_priv_t *)ip_hash);
                list_copy(lt, tmp);
                list_append(lt, ptr);
                for(unsigned int i = 0; i <= 255; i++){
                    if(i >= h && i <= t){
                        pptr[i] = lt;
                    }else{
                        pptr[i] = tmp;
                    }
                }
                free_mapping_table((ip_hash_priv_t *)ip_hash, *node);
            }
            *node = pmt;             
            pmt->ref_count++;
        }else if((*node)->type == MAPPING_256_TO_256){
            pptr = (*node)->mapping;
            list_t *lt = NULL;
            for(unsigned int i = h; i <= t; i++){
                if(pptr[i] != NULL){
                    list_append_unique(pptr[i], ptr, cmp_data, NULL);
                }else{
                    if(lt == NULL){
                        lt = malloc_list((ip_hash_priv_t *)ip_hash);
                        list_append(lt, ptr);                     
                    }
                    pptr[i] = lt;
                }                 
            }               
        }   
        return 0;
    }
    
    if(h != 0){
        if(*node == NULL || (*node)->type == MAPPING_256_TO_1){
            pmt = malloc_mapping_table((ip_hash_priv_t *)ip_hash, 256);
            if(pmt == NULL){
                return -1;
            }
            pmt->type = MAPPING_256_TO_256;
            pptr = pmt->mapping;
            
            if(*node == NULL){
                list_t *lt = malloc_list((ip_hash_priv_t *)ip_hash);
                list_append(lt, ptr);
                for(unsigned int i = h; i <= 255; i++){
                    pptr[i] = lt;
                }
            }else{
                pptr = (*node)->mapping;
                tmp = pptr[0];
                pptr = pmt->mapping;
                list_t *lt = malloc_list((ip_hash_priv_t *)ip_hash);
                list_copy(lt, tmp);
                list_append(lt, ptr);
                list_foreach(lt, print, NULL);
                for(unsigned int i = 0; i <= 255; i++){
                    if(i >= h)
                        pptr[i] = lt;
                    else
                        pptr[i] = tmp;
                }
                free_mapping_table((ip_hash_priv_t *)ip_hash, *node);
            }
            *node = pmt;             
            pmt->ref_count++;
        }else if((*node)->type == MAPPING_256_TO_256){
            pptr = (*node)->mapping;
            list_t *lt = NULL;
            for(unsigned int i = h; i <= 255; i++){
                if(pptr[i] != NULL){
                    list_append_unique(pptr[i], ptr, cmp_data, NULL);
                }else{
                    if(lt == NULL){
                        lt = malloc_list((ip_hash_priv_t *)ip_hash);
                        list_append(lt, ptr);                     
                    }
                    pptr[i] = lt;
                }
            }               
        }   
        s++;
    }

    node = HASH_KEY_TO_MP(ip_hash, e);
    if(t != 255){
       if(*node == NULL || (*node)->type == MAPPING_256_TO_1){
            pmt = malloc_mapping_table((ip_hash_priv_t *)ip_hash, 256);
            if(pmt == NULL){
                return -1;
            }
            pmt->type = MAPPING_256_TO_256;
            pptr = pmt->mapping;
            if(*node == NULL){
                list_t *lt = malloc_list((ip_hash_priv_t *)ip_hash);
                list_append(lt, ptr);
                for(unsigned int i = 0; i <= t; i++){
                    pptr[i] = lt;
                }
            }else{
                pptr = (*node)->mapping;
                tmp = pptr[0];
                pptr = pmt->mapping;
                list_t *lt = malloc_list((ip_hash_priv_t *)ip_hash);
                list_copy(lt, tmp);
                list_append(lt, ptr);
                for(unsigned int i = 0; i <= 255; i++){
                    if(i <= t)
                        pptr[i] = lt;
                    else
                        pptr[i] = tmp;
                }
                free_mapping_table((ip_hash_priv_t *)ip_hash, *node);
            }
            *node = pmt;     
            pmt->ref_count++;        
        }else if((*node)->type == MAPPING_256_TO_256){
            pptr = (*node)->mapping;
            list_t *lt = NULL;
            for(unsigned int i = 0; i <= t; i++){
                if(pptr[i] != NULL){
                    list_append_unique(pptr[i], ptr, cmp_data, NULL);
                }else{
                    if(lt == NULL){
                        lt = malloc_list((ip_hash_priv_t *)ip_hash);
                        list_append(lt, ptr);                     
                    }
                    pptr[i] = lt;
                }
            }               
        }   
        e--;
    }
    if(e<s)
        return 0;
    pmt = NULL;
    list_t *lt = NULL;
    for(unsigned int i = s; i <= e; i++){
        mapping_table **tmp = HASH_KEY_TO_MP(ip_hash, i);        
        mapping_table *mt = *tmp;
        if(mt == NULL){
            if(pmt == NULL){
                pmt = malloc_mapping_table((ip_hash_priv_t *)ip_hash, 1);  
                if(pmt == NULL){
                    return -1;
                }
                pmt->type = MAPPING_256_TO_1;
                pptr = pmt->mapping;
                if(lt == NULL){
                    lt = malloc_list((ip_hash_priv_t *)ip_hash);
                    list_append(lt, ptr);
                }
                pptr[0] = lt;           
            }
            *tmp = pmt;
            pmt->ref_count++;
        }else if(mt->type == MAPPING_256_TO_1){
            pptr = mt->mapping;
            list_append_unique(pptr[0], ptr, cmp_data, NULL);
        }else if(mt->type == MAPPING_256_TO_256){
            pptr = mt->mapping;
            for(unsigned int j = 0; j < 256; j++){
                if(pptr[j] != NULL){
                    list_append_unique(pptr[j], ptr, cmp_data, NULL);
                }else{
                    if(lt == NULL){
                        lt = malloc_list((ip_hash_priv_t *)ip_hash);
                        list_append(lt, ptr);
                    }
                    pptr[j] = lt;           
                }
            }
        }
    }
    return 0;
}

list_t * ip_hash_search(ip_hash_t *ip_hash, uint ip)
{
    void **pp;
    mapping_table **pmt = HASH_KEY_TO_MP(ip_hash, HASH_KEY(ip));
    mapping_table *mt = *pmt;
    //printf("node addr:%p, mt addr :%p\n", pmt, mt);
    if(ip_hash == NULL)
        return NULL;

    if(mt == NULL)
        return NULL;

    pp = mt->mapping;
    if(mt->type == MAPPING_256_TO_1){
        return pp[0];
    }else if(mt->type == MAPPING_256_TO_256){
        return pp[HASH_KEY_2(ip)];
    }else{
        printf("bad mapping table: unknow type\n");
        return NULL;
    }
}



