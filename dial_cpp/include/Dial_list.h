#ifndef __DIAL_LIST_H_
#define __DIAL_LIST_H_


#include <pthread.h>

typedef struct list_node {	
	 struct list_node *next;
	 struct list_node *prev;
} DIAL_LIST_NODE;


typedef struct list_head {
	DIAL_LIST_NODE head;
	int num;
	pthread_mutex_t lock;
}DIAL_LIST_HEAD;





#define INIT_HEAD(head) {(head).next = &(head);(head).prev = &(head);}



static inline void 
list_head_init(DIAL_LIST_HEAD *list)
{
	INIT_HEAD(list->head);
	list->num = 0;
	pthread_mutex_init(&(list->lock),NULL);
}

static inline void 
list_add(DIAL_LIST_HEAD *list,DIAL_LIST_NODE *node)
{
	pthread_mutex_lock(&(list->lock));

	node->next = list->head.next;
	node->prev = &list->head;
	(list->head.next)->prev = node;
	list->head.next = node;
	list->num++;
	
	pthread_mutex_unlock(&(list->lock));
}

static inline void 
list_del(DIAL_LIST_HEAD *list,DIAL_LIST_NODE *node)
{
	pthread_mutex_lock(&(list->lock));
	
 	node->next->prev = node->prev;
	node->prev->next = node->next;
	list->num--;
	
	pthread_mutex_unlock(&(list->lock));
}


/*@cur: current node of list_node 
 *@head: the head node of list */
#define list_for_each(cur, head) \
	for (cur = (head)->next; \
		(cur) != (head); \
		cur = (cur)->next)

/*@cur: current node of list_node 
 *@tmp:cur->next 
 *@head: the head node of list */
#define list_for_each_safe(cur, tmp, head) \
	for (cur = (head)->next, tmp = (cur)->next; \
		(cur) != (head); \
		cur = tmp, tmp = (tmp)->next)

#endif





