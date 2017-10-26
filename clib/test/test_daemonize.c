#include <clib/daemon.h>
#include <clib/array.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
void test_daemon(int argc, char **argv)
{
	if (argc == 2) {
        if (strcmp("start", argv[1]) == 0) {
            printf("starting daemon...\n");
            daemon_start(4);
        }
        else if (strcmp("stop", argv[1]) == 0) {
            printf("stoping daemon...\n");
            daemon_stop();
            exit(EXIT_SUCCESS);
        }
        else if (strcmp("restart", argv[1]) == 0) {
            printf("stoping daemon...\n");
            daemon_stop();
            printf("starting daemon...\n");
            daemon_start(4);
        }
    } else {
        printf("usuage: ./testclib start|stop|restart\n");
        exit(EXIT_FAILURE);
    }
	while (1) {
		sleep(2);
		printf("in baby process %d\n", getpid());
	}

}
