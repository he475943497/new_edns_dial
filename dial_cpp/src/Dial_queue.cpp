//采用单向链表
#include "Dial_queue.h"



void
queue_push(queue_info_t *info,DIAL_LIST_NODE *node)
{	
	pthread_mutex_lock(&info->lock);
	
	if(NULL == info->head) {			
		info->head = node;
		info->tail = info->head;
	} else {
		info->head->next = node;
		info->head = node;
	}
	info->cnt++;
	
	pthread_mutex_unlock(&info->lock);
}


DIAL_LIST_NODE *
queue_pop_and_del(queue_info_t *info)
{	
	DIAL_LIST_NODE *node = NULL;

	pthread_mutex_lock(&info->lock);
	
	if(NULL != info->tail) 
	{
		node = info->tail;
		info->tail = node->next;
		info->cnt--;
	}
	else
	{
		info->head = info->tail;
	}

	pthread_mutex_unlock(&info->lock);

	return node;
}

DIAL_LIST_NODE *
queue_pop(queue_info_t *info)
{	
	return queue_pop_and_del(info);
}





void
queue_init(queue_info_t *info)
{
	info->cnt = 0;
	info->head = NULL;
	info->tail = NULL;
	pthread_mutex_init(&info->lock,NULL);
}








