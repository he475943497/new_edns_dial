#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <clib/timer.h>

void handle(void *data)
{
    printf("call me:%d\n", *(int *)data);
}


int test_timer()
{
    int i = 60;
    timer_t timer_id;
    int ret = timer_new(&timer_id, (timer_handler)handle, &i);
    if (ret == -1) {
        printf("create timer fail\n");
        exit(-1);
    }

    timer_start(timer_id, 0, /*miro seconds=*/300 * 1000 * 1000);
    while (1) {
        usleep(300);
    }


    return 0;
//    int i = 60;
//    int ret;
//    timer_t timer_id;
//    ret = timer_new(&timer_id, (timer_handler)handle, &i);
//    if(ret == -1)
//	printf("error");		 
// redo:
//    i = 60;
//    printf("start\n");
//    timer_start(timer_id, 0, /*miro seconds=*/300 * 1000 * 1000);
//
//    while(i){
//	usleep(100);
//	i--;
//    } 
//
//    printf("stop\n");
//    timer_stop(timer_id);
//    i = 60;
//    while(i){
//	usleep(100);
//	i--;
//    }
//    goto redo;
//    timer_free(timer_id);
//   return 0;

}
