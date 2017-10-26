#include <clib/time.h>
#include <stdio.h>
void test_time()
{
    time_t now = time(NULL);
    printf("%ld\n", time_start_minute(now));
    printf("%ld\n", time_next_minute(now));

    printf("%ld\n", time_now_start_minute());
    printf("%ld\n", time_now_next_minute());

}
