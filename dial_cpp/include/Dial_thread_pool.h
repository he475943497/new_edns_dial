#ifndef DIAL_THREAD_POOL_
#define DIAL_THREAD_POOL_



#include <pthread.h>


typedef struct threadpool {

	int hope_threads_num;		//设置工作线程池线程数
	int act_threads_num;		//实际运行的线程数
	int threads_used;			//已在运行的线程数
	int threads_run_flag;		//工作线程运行标志
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

