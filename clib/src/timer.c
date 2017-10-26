#include "clib/timer.h"

#include <signal.h>
#include <time.h>



int timer_new(timer_t *timer_id, timer_handler func, void *data)
{
    int ret;
    struct sigevent sev;
   
    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_notify_function = func;
    sev.sigev_value.sival_ptr = data;
    sev.sigev_notify_attributes = NULL;
    ret = timer_create(CLOCK_REALTIME, &sev, timer_id);
    return ret;
}


int timer_start(timer_t timer_id, uint interval, long nano_sec)
{
    int ret;
    struct itimerspec its;
    its.it_value.tv_sec = interval;
    its.it_value.tv_nsec = nano_sec;

    its.it_interval.tv_sec = interval;
    its.it_interval.tv_nsec = nano_sec;

    ret = timer_settime(timer_id, 0, &its, NULL);
    return ret;
}

int timer_oneshot(timer_t timer_id, uint interval, long nano_sec)
{
    int ret;
    struct itimerspec its;
    its.it_value.tv_sec = interval;
    its.it_value.tv_nsec = nano_sec;

    //just one shot
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;

    ret = timer_settime(timer_id, 0, &its, NULL);
    return ret;
}

int timer_stop(timer_t timer_id)
{
    int ret;
    struct itimerspec its;
    ret = timer_gettime(timer_id, &its);
    if(ret == -1)
	return ret;
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = 0;
    ret = timer_settime(timer_id, 0, &its, NULL);
    return ret;
}

int timer_free(timer_t timer_id)
{
    return timer_delete(timer_id);
}
