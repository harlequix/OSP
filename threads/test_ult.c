#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include "ult.h"
int end_copying;
long copyed_bytes;
time_t start_time;
void print_stack_pointer( char *info );

void threadA()
{
	print_stack_pointer("Thread A");
	char out[129];
	//file benutzen
	int  handle = open("/dev/random", O_RDONLY);
	if(ult_read(handle,out,10)){
		printf("gelesen:%s\n",out);
	}
	ult_yield();
	
	  printf("works!\n");
	  ult_exit(0);
	
	
}
void threadB()
{
  print_stack_pointer("Thread B");
	printf("blub\n");
	ult_exit(0);
}


void myInit()
{
    print_stack_pointer("Init");
    int cpid[2],i, status;
    printf("spawn A\n"); fflush(stdout);
    cpid[0] = ult_spawn(threadA);
    //printf("spawn B\n"); fflush(stdout);
    int foo =4;
    if(foo==4);
    cpid[1] = ult_spawn(threadB);
    //print_queue();
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
void print_stack_pointer( char *info )
{
  long stackp = 0;
  long basep = 0;

  __asm__ __volatile__ ( "mov %%ebp, %%eax":"=a" (basep));
  __asm__ __volatile__ ( "mov %%esp, %%eax":"=a" (stackp));
  
  printf("%s Stackpointer : %lu Basepointer: %lu\n",info,stackp,basep);

  return;
}

