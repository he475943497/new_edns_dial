#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "Dial_list.h"
#include "Dial_hash.h"
#include "Dial_server.h"
#include "Dial_common.h"


void 
hash_del(hash_info_t *hash,
	DIAL_LIST_NODE *node)
{


}

int  
hash_key_healthpolicy(void *str,int num)
{
	unsigned int rtn = 0;
	unsigned int key = 0;
	char *pstr = (char *)str;
	int lenth = 0;	
	while(*pstr != '\0') {
		rtn += *pstr++;
		lenth++;
	}

	//return ((rtn*lenth)%(num-1));
	key = (rtn*lenth)%(num-1);
	return key;
}

int  
hash_key_nginxgroup(void *src,int num)
{
	/*unsigned int rtn = 0;
	int lenth = 0;	
	unsigned int key = 0;
	nginxgroup_info_t *cur = (nginxgroup_info_t *)src;
	
	char *pstr = cur->name;
	int lenth = 0;	
	while(*pstr != '\0') {
		rtn += *pstr++;
		lenth++;
	}

	key = (rtn*lenth)%(num-1);

	return key;*/

	unsigned int rtn = 0;
	unsigned int key = 0;
	char *pstr = (char *)src;
	int lenth = 0;	
	while(*pstr != '\0') {
		rtn += *pstr++;
		lenth++;
	}

	//return ((rtn*lenth)%(num-1));
	key = (rtn*lenth)%(num-1);
	return key;
	
}


int  
hash_key_healthgroup(void *src,int num)
{
	unsigned int rtn = 0;
	int lenth = 0;	
	unsigned int key = 0;
	healthgroup_info_t *cur = (healthgroup_info_t *)src;
	
	char *pstr = cur->viewname;
	while(*pstr != '\0') {
		rtn += *pstr++;
		lenth++;
	}
	
	/*char *pstr = cur->zonename;	
	while(*pstr != '\0') {
		rtn += *pstr++;
		lenth++;
	}*/
	
	pstr = cur->name;	
	while(*pstr != '\0') {
		rtn += *pstr++;
		lenth++;
	}	
	
	//return ((rtn*lenth)%(num-1));
	key = (rtn*lenth)%(num-1);
	return key;
}


void 
hash_add(hash_info_t *hash,void *src,DIAL_LIST_NODE *node)
{
	int key = hash->hash_key(src,hash->num);
	//debug_printf("hash_add:src=%s,key=%d\n",src,key);
	list_add(&hash->tab[key],node);
//	debug_printf("2222222222\n");
	
}



DIAL_LIST_NODE * 
hash_search_healthpolicy(hash_info_t *hash,
				char *src)
{
	DIAL_LIST_NODE *cur = NULL;
	DIAL_LIST_NODE *head = NULL;
	DIAL_LIST_HEAD *head_node = NULL;
	healthpolicy_info_t *tmp = NULL;
	
	int key = hash->hash_key(src,hash->num);
	//debug_printf("hash_search_healthpolicy:key=%d\n",key);
	//cur = hash->tab[key].head.next;
	head = &hash->tab[key].head;
	head_node = &hash->tab[key];
	
	pthread_mutex_lock(&head_node->lock);
	
	list_for_each(cur, head) {
		tmp = (healthpolicy_info_t *)cur;
		if(!strcmp(src,tmp->policyname)) {
			break;
		}
	}
	pthread_mutex_unlock(&head_node->lock);
	
	if(cur == head)
		return NULL;
	else
		return cur;
}

DIAL_LIST_NODE * 
hash_search_nginxgroup(hash_info_t *hash,
				char *src)
{
	DIAL_LIST_NODE *cur = NULL;
	DIAL_LIST_NODE *head = NULL;
	DIAL_LIST_HEAD *head_node = NULL;
	nginxgroup_info_t *tmp = NULL;
	
	int key = hash->hash_key(src,hash->num);
	//debug_printf("hash_search_healthpolicy:key=%d\n",key);
	//cur = hash->tab[key].head.next;
	head = &hash->tab[key].head;
	head_node = &hash->tab[key];
	
	pthread_mutex_lock(&head_node->lock);
	
	list_for_each(cur, head) {
		tmp = (nginxgroup_info_t *)cur;
		if(!strcmp(src,tmp->name)) {
			break;
		}
	}
	pthread_mutex_unlock(&head_node->lock);
	
	if(cur == head)
		return NULL;
	else
		return cur;
}


DIAL_LIST_NODE * 
hash_search_healthgroup(hash_info_t *hash,
				char *src)
{
	DIAL_LIST_NODE *cur = NULL;
	DIAL_LIST_NODE *head = NULL;
	DIAL_LIST_HEAD *head_node = NULL;
	healthgroup_info_t *tmp = NULL;
	healthgroup_info_t *hg = (healthgroup_info_t *)src;
	
	int key = hash->hash_key(src,hash->num);
	//debug_printf("hash_search_healthgroup:key=%d\n",key);
	//cur = hash->tab[key].head.next;
	head = &hash->tab[key].head;
	head_node = &hash->tab[key];
	
	pthread_mutex_lock(&head_node->lock);
	
	list_for_each(cur, head) {
		tmp = (healthgroup_info_t *)cur;
		if(!strcmp(hg->viewname,tmp->viewname) && !strcmp(hg->zonename,tmp->zonename) && !strcmp(hg->name,tmp->name)) {
			break;
		}
	}
	pthread_mutex_unlock(&head_node->lock);
	
	if(cur == head){
		debug_printf(LOG_LEVEL_ERROR,"hash_search_healthgroup:can not find code,dn=%s,key=%d\n",hg->name,key);
		
		return NULL;
	}
	else
		return cur;
}


void 
hash_destory_healthgroup(hash_info_t *hash)
{
	int i;
	DIAL_LIST_NODE *cur = NULL;
	DIAL_LIST_NODE *tmp = NULL;
	DIAL_LIST_NODE *head =NULL;
	DIAL_LIST_NODE *cur_r = NULL;
	DIAL_LIST_NODE *tmp_r= NULL;
	DIAL_LIST_NODE *head_r=NULL;
	DIAL_LIST_NODE *cur_p = NULL;
	DIAL_LIST_NODE *tmp_p= NULL;
	DIAL_LIST_NODE *head_p=NULL;
	DIAL_LIST_HEAD *phead = NULL;
	healthgroup_info_t *hg = NULL;
	record_info_t  *precord = NULL;
	pPolicy_node_t *pPolicy = NULL;
	
	for(i = 0; i < hash->num; ++i) {
		cur = NULL;
		head = &(hash->tab[i].head);
		phead = &hash->tab[i];
		
		pthread_mutex_lock(&phead->lock);
		list_for_each_safe(cur,tmp,head) {
			hg = (healthgroup_info_t *)cur;
			
			//free record nodes
			head_r = &hg->record_head.head;
			list_for_each_safe(cur_r,tmp_r,head_r) {
				precord = (record_info_t *)cur_r;				
				list_del(&hg->record_head,cur_r);
				free(precord);
			}
			
			//free policy nodes
			head_p = &hg->pPolicy_head.head;
			list_for_each_safe(cur_p,tmp_p,head_p) {
				pPolicy = (pPolicy_node_t *)cur_p;				
				list_del(&hg->pPolicy_head,cur_p);
				free(pPolicy);
			}	
			
			//free health group nodes
			list_del(&hash->tab[i],cur);
			free(hg);
		}
		pthread_mutex_unlock(&phead->lock);
	}

	free(hash->tab);
	
	hash->num = 0;
	hash->tab = NULL;
	hash->hash_key = NULL;
	hash->hash_add = NULL;
	hash->hash_del = NULL;
	hash->hash_clear = NULL;
	hash->hash_search = NULL;

}

void 
hash_destory_nginxgroup(hash_info_t *hash)
{
	int i;
	DIAL_LIST_NODE *cur = NULL;
	DIAL_LIST_NODE *tmp = NULL;
	DIAL_LIST_NODE *head =NULL;
	DIAL_LIST_NODE *cur_srv = NULL;
	DIAL_LIST_NODE *tmp_srv= NULL;
	DIAL_LIST_NODE *head_srv=NULL;
	DIAL_LIST_NODE *cur_p = NULL;
	DIAL_LIST_NODE *tmp_p= NULL;
	DIAL_LIST_NODE *head_p=NULL;
	DIAL_LIST_HEAD *phead = NULL;
	nginxgroup_info_t *ng = NULL;
	nginx_srv_t  *psrv = NULL;
	pPolicy_node_t *pPolicy = NULL;	
	
	for(i = 0; i < hash->num; ++i) {
		cur = NULL;
		head = &(hash->tab[i].head);
		phead = &hash->tab[i];
		
		pthread_mutex_lock(&phead->lock);

		list_for_each_safe(cur,tmp,head) {
			ng = (nginxgroup_info_t *)cur;
			
			//free record nodes
			head_srv = &ng->srv_head.head;
			list_for_each_safe(cur_srv,tmp_srv,head_srv) {
				psrv = (nginx_srv_t *)cur_srv;				
				list_del(&ng->srv_head,cur_srv);
				free(psrv);
			}
				
			//free policy nodes
			head_p = &ng->pPolicy_head.head;
			list_for_each_safe(cur_p,tmp_p,head_p) {
				pPolicy = (pPolicy_node_t *)cur_p;				
				list_del(&ng->pPolicy_head,cur_p);
				free(pPolicy);
			}	
					
			//free health group nodes
			list_del(&hash->tab[i],cur);
			free(ng);
		}
		pthread_mutex_unlock(&phead->lock);
	}

	free(hash->tab);
	
	hash->num = 0;
	hash->tab = NULL;
	hash->hash_key = NULL;
	hash->hash_add = NULL;
	hash->hash_del = NULL;
	hash->hash_clear = NULL;
	hash->hash_search = NULL;

}


void 
hash_destory_healthpolicy(hash_info_t *hash)
{
	int i;
	DIAL_LIST_NODE *cur = NULL;
	DIAL_LIST_NODE *tmp = NULL;
	DIAL_LIST_NODE *head =NULL;
	DIAL_LIST_HEAD *phead = NULL;
	DIAL_LIST_NODE *cur_code = NULL;
	DIAL_LIST_NODE *tmp_code = NULL;
	DIAL_LIST_NODE *head_code =NULL;
	healthpolicy_info_t *hp = NULL;
	match_code_t	*mcode = NULL;
		
	for(i = 0; i < hash->num; ++i) {
		cur = NULL;
		head = &(hash->tab[i].head);
		phead = &hash->tab[i];
		
		pthread_mutex_lock(&phead->lock);

		list_for_each_safe(cur,tmp,head) {
			hp = (healthpolicy_info_t *)cur;

			head_code = &hp->dial_option.code_head.head;
			list_for_each_safe(cur_code,tmp_code,head_code) {
				mcode = (match_code_t	*)cur_code;
				list_del(&hp->dial_option.code_head,&mcode->node);
				free(mcode);
			}
			
			//free health policy nodes
			list_del(&hash->tab[i],cur);
			free(hp);
		}
		pthread_mutex_unlock(&phead->lock);
	}

	free(hash->tab);
	
	hash->num = 0;
	hash->tab = NULL;
	hash->hash_key = NULL;
	hash->hash_add = NULL;
	hash->hash_del = NULL;
	hash->hash_clear = NULL;
	hash->hash_search = NULL;

}

void 
hash_clear_healthgroup(hash_info_t *hash)
{
	int i;
	DIAL_LIST_NODE *cur = NULL;
	DIAL_LIST_NODE *tmp = NULL;
	DIAL_LIST_NODE *head =NULL;
	DIAL_LIST_NODE *cur_r = NULL;
	DIAL_LIST_NODE *tmp_r= NULL;
	DIAL_LIST_NODE *head_r=NULL;	
	DIAL_LIST_NODE *cur_p = NULL;
	DIAL_LIST_NODE *tmp_p= NULL;
	DIAL_LIST_NODE *head_p=NULL;
	DIAL_LIST_HEAD *phead = NULL;
	healthgroup_info_t *hg = NULL;
	record_info_t  *precord = NULL;
	pPolicy_node_t *pPolicy = NULL;
	
	for(i = 0; i < hash->num; ++i) {
		cur = NULL;
		head = &(hash->tab[i].head);
		phead = &hash->tab[i];
		
		pthread_mutex_lock(&phead->lock);

		list_for_each_safe(cur,tmp,head) {
			hg = (healthgroup_info_t *)cur;
			
			//free record nodes
			head_r = &hg->record_head.head;
			list_for_each_safe(cur_r,tmp_r,head_r) {
				precord = (record_info_t *)cur_r;				
				list_del(&hg->record_head,cur_r);
				free(precord);
			}
			//free policy nodes
			head_p = &hg->pPolicy_head.head;
			list_for_each_safe(cur_p,tmp_p,head_p) {
				pPolicy = (pPolicy_node_t *)cur_p;				
				list_del(&hg->pPolicy_head,cur_p);
				free(pPolicy);
			}	
			//free health group nodes
			list_del(&hash->tab[i],cur);
			free(hg);
		}
		pthread_mutex_unlock(&phead->lock);
	}

}

void 
hash_clear_nginxgroup(hash_info_t *hash)
{
	int i;
	DIAL_LIST_NODE *cur = NULL;
	DIAL_LIST_NODE *tmp = NULL;
	DIAL_LIST_NODE *head =NULL;
	DIAL_LIST_NODE *cur_srv = NULL;
	DIAL_LIST_NODE *tmp_srv= NULL;
	DIAL_LIST_NODE *head_srv=NULL;
	DIAL_LIST_NODE *cur_p = NULL;
	DIAL_LIST_NODE *tmp_p= NULL;
	DIAL_LIST_NODE *head_p=NULL;
	DIAL_LIST_HEAD *phead = NULL;
	nginxgroup_info_t *ng = NULL;
	nginx_srv_t  *psrv = NULL;
	pPolicy_node_t *pPolicy = NULL;

	
	for(i = 0; i < hash->num; ++i) {
		cur = NULL;
		head = &(hash->tab[i].head);
		phead = &hash->tab[i];
		
		pthread_mutex_lock(&phead->lock);

		list_for_each_safe(cur,tmp,head) {
			ng = (nginxgroup_info_t *)cur;
			
			//free record nodes
			head_srv = &ng->srv_head.head;
			list_for_each_safe(cur_srv,tmp_srv,head_srv) {
				psrv = (nginx_srv_t *)cur_srv;				
				list_del(&ng->srv_head,cur_srv);
				free(psrv);
			}
					
			//free policy nodes
			head_p = &ng->pPolicy_head.head;
			list_for_each_safe(cur_p,tmp_p,head_p) {
				pPolicy = (pPolicy_node_t *)cur_p;				
				list_del(&ng->pPolicy_head,cur_p);
				free(pPolicy);
			}	
						
			//free health group nodes
			list_del(&hash->tab[i],cur);
			free(ng);
		}
		pthread_mutex_unlock(&phead->lock);
	}

}



void 
hash_clear_healthpolicy(hash_info_t *hash)
{
	int i;
	DIAL_LIST_NODE *cur = NULL;
	DIAL_LIST_NODE *tmp = NULL;
	DIAL_LIST_NODE *head =NULL;
	DIAL_LIST_HEAD *phead = NULL;
	DIAL_LIST_NODE *cur_code = NULL;
	DIAL_LIST_NODE *tmp_code = NULL;
	DIAL_LIST_NODE *head_code =NULL;
	healthpolicy_info_t *hp = NULL;
	match_code_t	*mcode = NULL;
	
	for(i = 0; i < hash->num; ++i) {
		cur = NULL;
		head = &(hash->tab[i].head);
		phead = &hash->tab[i];
		
		pthread_mutex_lock(&phead->lock);

		list_for_each_safe(cur,tmp,head) {
			hp = (healthpolicy_info_t *)cur;
			head_code = &hp->dial_option.code_head.head;
			
			list_for_each_safe(cur_code,tmp_code,head_code) {
				mcode = (match_code_t	*)cur_code;
				list_del(&hp->dial_option.code_head,&mcode->node);
				free(mcode);
			}
			
			//free health policy nodes
			list_del(&hash->tab[i],cur);
			free(hp);
		}
		pthread_mutex_unlock(&phead->lock);
	}

}

int
hash_init_healthgroup(hash_info_t *hash,
	int num)
{
	int i;
	hash->num = num;
	hash->hash_key = hash_key_healthgroup;
	hash->hash_add = hash_add;
	hash->hash_search = hash_search_healthgroup;
	hash->hash_clear= hash_clear_healthgroup;	
	hash->tab = (DIAL_LIST_HEAD *)malloc(sizeof(DIAL_LIST_HEAD)*num);
	if(NULL == hash->tab)
		return -1;
	for(i = 0; i < num;++i) {
		list_head_init(&hash->tab[i]);
	}

	return 0;
}

int
hash_init_nginxgroup(hash_info_t *hash,
	int num)
{
	int i;
	hash->num = num;
	hash->hash_key = hash_key_nginxgroup;
	hash->hash_add = hash_add;
	hash->hash_search = hash_search_nginxgroup;
	hash->hash_clear= hash_clear_nginxgroup;	
	hash->tab = (DIAL_LIST_HEAD *)malloc(sizeof(DIAL_LIST_HEAD)*num);
	if(NULL == hash->tab)
		return -1;
	for(i = 0; i < num;++i) {
		list_head_init(&hash->tab[i]);
	}

	return 0;
}

int
hash_init_healthpolicy(hash_info_t *hash,
	int num)
{
	int i;
	hash->num = num;
	hash->hash_key = hash_key_healthpolicy;
	hash->hash_add = hash_add;
	hash->hash_search = hash_search_healthpolicy;
	hash->hash_clear= hash_clear_healthpolicy;	
	hash->tab = (DIAL_LIST_HEAD *)malloc(sizeof(DIAL_LIST_HEAD)*num);
	if(NULL == hash->tab)
		return -1;
	for(i = 0; i < num;++i) {
		list_head_init(&hash->tab[i]);
	}

	return 0;
}





