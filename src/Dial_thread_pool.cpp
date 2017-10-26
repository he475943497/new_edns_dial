#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include "Dial_server.h"
#include "Dial_queue.h"
#include "Dial_thread_pool.h"
#include "Dial_common.h"


void *
threadpool_worker_thread(void *arg)
{
		extern queue_info_t		queue_info;
		threadpool_t *tp = (threadpool_t *)arg;
		DIAL_LIST_NODE *node = NULL;

START:	
	while(tp->threads_run_flag) {

		dial_node_t *task_node = NULL;

		pthread_mutex_lock(&tp->mutex);
		pthread_cond_wait(&tp->cond,&tp->mutex);
		pthread_mutex_unlock(&tp->mutex);
		
		if (tp->threads_run_flag==0) 
			break;
		
		pthread_mutex_lock(&tp->lock);
		tp->threads_used++;
		pthread_mutex_unlock(&tp->lock);

		while(NULL != (node = queue_pop(&queue_info))) {
			debug_printf(LOG_LEVEL_DEBUG,"_______receive a task!!\n");
			task_node = (dial_node_t *)node;
			if(task_node->type == HEALTHGROUP) {
				do_a_dial_healthgroup(task_node->dial_node.healthgroup);
				free(task_node->dial_node.healthgroup);	
			} else if(task_node->type == SERVER) {
				do_a_dial_server(task_node->dial_node.srv);
				free(task_node->dial_node.srv);
			}else if(task_node->type == NGINX) {
				do_a_dial_nginxgroup(task_node->dial_node.nginxgroup);
				free(task_node->dial_node.nginxgroup);
			}
			
			free(task_node);
			
			if(0 == tp->threads_run_flag)
				break;
		}
				
		pthread_mutex_lock(&tp->lock);
		tp->threads_used--;
		pthread_mutex_unlock(&tp->lock);

	}
	
	pthread_mutex_lock(&tp->lock);
	tp->act_threads_num--;
	pthread_mutex_unlock(&tp->lock);
	
}

int 
threadpool_init(threadpool_t *tp,
				int hope_threads_num)
{

	int i;
	int ret;

	tp->hope_threads_num = hope_threads_num;
	tp->threads_used = 0;
	tp->worker_thread_ids = (pthread_t *)malloc(hope_threads_num*sizeof(pthread_t));
	tp->threads_run_flag = 1;
	tp->act_threads_num = 0;
		
	if ((ret = pthread_mutex_init(&tp->mutex,NULL))!=0) 
		return ret;
	if ((ret = pthread_mutex_init(&tp->lock,NULL))!=0) 
		return ret;

	if ((ret = pthread_cond_init(&tp->cond,NULL))!=0) 
		return ret;

	for(i=0;i<hope_threads_num;i++)
	{

	    if ((ret = pthread_create(&tp->worker_thread_ids[i],NULL,threadpool_worker_thread,tp))!=0) 
			return ret;

	    tp->act_threads_num++;
	}

	return 0;

}

int 
threadpool_destroy(threadpool_t *tp)
{
	int i = 0;
	tp->threads_run_flag = 0;

	pthread_cond_broadcast(&tp->cond);

	while(tp->act_threads_num>0)
	{
	    usleep(1*1000);
		pthread_cond_broadcast(&tp->cond);

	}
	for(i=0;i<tp->hope_threads_num;i++)
	{

		pthread_join(tp->worker_thread_ids[i],NULL);
	}
	
	free(tp->worker_thread_ids);

	return 0;
	
}









