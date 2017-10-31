#ifndef DIAL_QUEUE_
#define DIAL_QUEUE_

#include "Dial_list.h"

typedef struct queue_node_info {
	DIAL_LIST_NODE *head;
	DIAL_LIST_NODE *tail;
	int cnt;
	pthread_mutex_t lock;
} queue_info_t;



void
queue_init(queue_info_t *info);
DIAL_LIST_NODE *
queue_pop(queue_info_t *info);
void
queue_push(queue_info_t *info,DIAL_LIST_NODE *node);


#endif

