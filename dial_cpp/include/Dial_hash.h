#ifndef DIAL_HASH_H

#define DIAL_HASH_H




//hash 表结构信息
typedef struct hash_info {
	int num;		//tab 大小
	DIAL_LIST_HEAD *tab;
	int (*hash_key)(void *,int);
	void (*hash_add)(struct hash_info *,void *,DIAL_LIST_NODE *);
	void (*hash_del)(struct hash_info *,DIAL_LIST_NODE *);
	void (*hash_clear)(struct hash_info *);
	DIAL_LIST_NODE *(*hash_search)(struct hash_info *,char *);
	
} hash_info_t;

int
hash_init_healthgroup(hash_info_t *hash,
	int num);
int
hash_init_nginxgroup(hash_info_t *hash,
	int num);
int
hash_init_healthpolicy(hash_info_t *hash,
	int num);
void 
hash_destory_healthgroup(hash_info_t *hash);
void 
hash_destory_nginxgroup(hash_info_t *hash);
void 
hash_destory_healthpolicy(hash_info_t *hash);

#endif








