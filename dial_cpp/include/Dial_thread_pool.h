#ifndef DIAL_THREAD_POOL_
#define DIAL_THREAD_POOL_



#include <pthread.h>


typedef struct threadpool {

	int hope_threads_num;		//���ù����̳߳��߳���
	int act_threads_num;		//ʵ�����е��߳���
	int threads_used;			//�������е��߳���
	int threads_run_flag;		//�����߳����б�־
	pthread_t *worker_thread_ids;
	pthread_mutex_t mutex,lock;
	pthread_cond_t cond;
}threadpool_t;




void *threadpool_worker_thread(void *tp);
int threadpool_destroy(threadpool_t *tp);
int 
threadpool_init(threadpool_t *tp,
				int hope_threads_num);


#endif

