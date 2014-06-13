#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include "ult.h"

int end_copying;
long copyed_bytes;
time_t start_time;

void threadA()
{
	ult_exit(0);
}

void threadB()
{
	ult_exit(0);
}

void myInit()
{
    int cpid[2], i, status;
    printf("spawn A\n"); fflush(stdout);
    cpid[0] = ult_spawn(threadA);
    printf("spawn B\n"); fflush(stdout);
    cpid[1] = ult_spawn(threadB);
    for (i = 0; i < 2; i++) {
        printf("waiting for cpid[%d] = %d\n", i, cpid[i]); fflush(stdout);
        if (ult_waitpid(cpid[i], &status) == -1) {
            fprintf(stderr, "waitpid for %d failed\n", cpid[i]);
            ult_exit(-1);
        }
        printf("(status = %d)\n", status);
    }
    puts("ciao!");
    ult_exit(0);
}

int  main()
{
    printf("starting myInit\n"); fflush(stdout);
    ult_init(myInit);
    exit(0);
}


