#define DEBUG 0

#include <signal.h>
#include <setjmp.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "ult.h"
#include "tcb.h"
#include <sys/queue.h>
#define _BSD_SOURCE 1
#define STACK 1024*64
static int id=0;

struct tailque_entry{
 	tcb context;
 	TAILQ_ENTRY(tailque_entry) entries;
 	//TODO: eventually unifying tcb and tailque_entry
};
void print_stack_pointer( char *info );
//Deklaration der Queues
TAILQ_HEAD(,tailque_entry) running_queue;
TAILQ_HEAD(,tailque_entry) blocking_queue;
TAILQ_HEAD(,tailque_entry) zombie_queue;
jmp_buf init,spawn_buf,spawn_hack, spawn_hack_2, ult_sp;
void is_needed_by_process(int id);
void schedule();
ult_func global_funct_ptr;
fd_set set;
int jmpinfo;


/*
 This function only exists to tell the process to use an empty stack for the thread
 */

void signalHandlerSpawn( ){	
	//ult_func handler_func_ptr= global_funct_ptr;
	//printf("Funtion Pointer vor Sprung: %p\n", handler_func_ptr );
	print_stack_pointer("Stackpointer vor dem Sprung");
	if(setjmp(spawn_buf)){
		global_funct_ptr();
		longjmp(init,1);
	}
	return;
	
}


// spawn a new thread, return its ID 
/*	Die Funktion ult_spawn() erzeugt einen Thread-Control-Block (TCB) fuer
 die Funktion f() und stellt den Thread in die Run-Queue. Sie liefert
 den ID des erzeugten Threads zurueck. An dieser Stelle erfolgt kein
 Scheduling, sondern die Abarbeitung wird mit dem laufenden Thread
 fortgesetzt, um die Erzeugung mehrerer Threads zu ermoeglichen.
 */


int ult_spawn(ult_func f) {
  id++;
  global_funct_ptr=f;
  if(setjmp(ult_sp)){
    return 0;
  }
  else{
    printf("jumping to spawn hack");
    longjmp(spawn_hack,1);
  }
}
//dirty^n hack; n \to \infty
void ult_spawn_hack(){
	if(setjmp(spawn_hack)){
	struct sigaction sa;
	stack_t stack;
	tcb *spawn;
	spawn=(tcb*)malloc(sizeof(tcb));
  	spawn->id=id;
  	printf("Setting up stack\n");
	// Create the new stack
 	stack.ss_flags = 0;
 	stack.ss_size = STACK;
  	stack.ss_sp = malloc(STACK );
  	if ( stack.ss_sp == 0 ) {
    	perror( "Could not allocate stack." );
    	exit( 1 );
  	}
	if (sigaltstack(&stack, NULL) == -1){
	  perror("error");
	}
	
  	//printf("Altering Signal\n");
  	long unsigned int foo =(long unsigned int) stack.ss_sp;
  	printf("%lu\n", foo );
	
  	sa.sa_flags = SA_ONSTACK;
  	sa.sa_handler=&signalHandlerSpawn;
  	sigemptyset( &sa.sa_mask );
	
  	sigaction( SIGUSR1, &sa, 0 );
  	

	
  	//Stack is somewhat broken, so we need to reload funktion pointer
  	//global_funct_ptr=f;
	
  	raise( SIGUSR1 );
	
  	//printf("back from signalhandler\n");
  	//printf("Saving tcb and pushing into the queue\n");
	
  	memcpy(spawn->context, spawn_buf, sizeof(jmp_buf));
  	spawn->function=global_funct_ptr;
  	spawn->id=id;
  	spawn->status=0;
  	spawn->stack=stack;
	struct tailque_entry *item;
	item = (struct tailque_entry*) malloc(sizeof(struct tailque_entry));
	item->context=*spawn;
	TAILQ_INSERT_TAIL(&running_queue, item, entries);
	longjmp(ult_sp,1);
	}
	else{
	  printf("Setting up spawn hack");
	}
	
	
			
}


// yield the CPU to another thread
/*	Die Funktion ult_yield() gibt innerhalb eines Threads freiwillig den
 Prozessor auf. Es erfolgt der Aufruf des Schedulers und die Umschaltung
 auf den vom Scheduler ausgewaehlten Thread.
 */
void ult_yield() {
	struct tailque_entry *tcb = TAILQ_FIRST(&running_queue);
	if (setjmp(tcb->context.context)) {
		printf("back in yield\n");
		return;
	}
	else {
		//print_stack_pointer("Stack yield:");
		//TODO: check for identical process hereafter (or if running_queue only continues only on )
		TAILQ_REMOVE(&running_queue, tcb, entries);
		TAILQ_INSERT_TAIL(&running_queue, tcb, entries);
		//print_queue();
		//printf("FOOOOOOBAR\n");
		longjmp(init,1);
	  
	}
	
}



// current thread exits
/*	Wird innerhalb eines Threads ult_exit() aufgerufen, so wird der Thread
 zum Zombie und der Exit-Status wird abgespeichert.
 */
void ult_exit(int status) {
	struct tailque_entry *tcb = TAILQ_FIRST(&running_queue);
	status=tcb->context.status;
	
	TAILQ_REMOVE(&running_queue, tcb, entries);
	TAILQ_INSERT_HEAD(&zombie_queue, tcb, entries);
	is_needed_by_process(tcb->context.id);
	//print_queue();
	//printf("FOOOOOOBAR\n");
	longjmp(init,1);
}


// thread waits for termination of another thread
// returns 0 for success, -1 for failure
// exit-status of terminated thread is obtained
/*	Mit ult_waitpid() kann abgefragt werden, ob der Thread mit dem angegebenen
 ID bereits beendet wurde. Ist der Thread bereits beendet, so kehrt die
 Funktion sofort zurueck und liefert in status den Exit-Status des Threads
 (welcher als Argument an ult_exit() uebergeben wurde). Laeuft der Thread noch,
 so soll der aktuelle Thread blockieren und es muss auf einen anderen Thread
 umgeschaltet werden. Bei nachfolgenden Aufrufen des Schedulers soll ueberprueft
 werden, ob der Thread mittlerweile beendet wurde; ist dies der Fall, so soll
 der aktuelle Thread geweckt werden (am besten so, dass er auch als naechster
 Thread die CPU erhaelt).
 */
int ult_waitpid(int tid, int *status) {
	struct tailque_entry *blocked = TAILQ_FIRST(&running_queue);
	struct tailque_entry *pid;
	printf("I'm in waitpid\n");
	print_queue();
	if(tid>id){
	  return -1;
	}
	  if(!setjmp(blocked->context.context)){
	    TAILQ_FOREACH(pid,&zombie_queue,entries){
		  if(pid->context.id==tid){
			  *status=pid->context.status;
			  return 0;
		  }
	    }
	    blocked->context.is_waiting_for=tid;
	    printf("Blocking Thread:%d \n", blocked->context.id);
	    print_queue();
	    TAILQ_REMOVE(&running_queue, blocked, entries);
	    
	    TAILQ_INSERT_HEAD(&blocking_queue, blocked, entries);
	    print_queue();
	    longjmp(init,1);
	  }
	  else{
	    return 0;
	  }
}




// read from file, block the thread if no data available
/*	Hinter ult_read() verbirgt sich die Funktion read() der Unix-API, welche Daten
 aus einer Datei (File-Deskriptor fd) in einen Puffer (buf) liest. Die Funktion
 ult_read() ist eine Wrapper-Funktion fuer read(), welche zusaetzlich ueberprueft, ob
 ausreichend Daten verfuegbar sind. Ist dies nicht der Fall, so wird der Thread
 blockiert und ein anderer Thread erhaelt die CPU. Bei nachfolgenden Aufrufen des
 Schedulers soll ueberprueft werden, ob mittlerweile ausreichend Daten vorliegen; ist
 dies der Fall, so soll der aktuelle Thread geweckt werden (s.o.). Dies loest ein
 Problem mit Systemrufen, welche im Kernel blockieren koennen (wie z.B. read()).
 Ohne diesen Mechanismus wuerde die gesamte User-Level-Thread-Bibliothek, die aus Sicht
 des Kernels ein Prozess ist, blockieren (selbst wenn andere User-Level-Threads
 lauffaehig waeren). Wir kuemmern uns hier nur um die read()-Funktion, obwohl auch andere
 Systemrufe blockieren koennen.
 */
int ult_read(int fd, void *buf, int count) {
	jmp_buf read_buf;
	struct timeval timeout;
	int vorhanden;
	size_t counts=count;
	
	//setjmp um spŠter wieder hierher zu kommen wenn gelesen werden kann
	setjmp(read_buf);
	
	//timeout time
	timeout.tv_sec = 0;
	timeout.tv_usec = 50;
	
	//fd in set makieren
	if (!FD_ISSET(fd, &set)) {
		FD_SET(fd, &set);       
	}
	
	//sind Daten zum lesen vorhanden?
	vorhanden = select(fd + 1, &set, NULL, NULL, &timeout);
	
	//wenn vorhanden lesen sonst Thread in waitqueue
	if (vorhanden) {
		read(fd, buf, counts);  
		
		FD_CLR(fd, &set);
		return 1;
	} 
	else {
		struct tailque_entry *tcb = TAILQ_FIRST(&running_queue);
		TAILQ_REMOVE(&running_queue, tcb, entries);
		//rŸcksprung Šndern, damit read erneut versucht wird und nicht ganze Funktion neu
		memcpy(tcb->context.context, read_buf, sizeof(jmp_buf));
		//*tcb->context.context=*read_buf;
		TAILQ_INSERT_TAIL(&blocking_queue, tcb, entries);		
		longjmp(init,1);
	}	
	return 0;	
}



// start scheduling and spawn a thread running function f
/*	Die Funktion ult_init() initialisiert die Bibliothek (insbesondere die Datenstrukturen
 des Schedulers wie z.B. Thread-Listen) und muss vor allen anderen Funktionen aufgerufen
 werden. Sie erzeugt einen einzigen Thread (analog zum Init-Thread‰ bei Unix), aus
 welchem heraus dann alle anderen Threads erzeugt werden und welcher danach mit
 ult_waitpid() auf das Ende aller Threads wartet. 
 */

void ult_init(ult_func f) {
	
	//printf("Setting up queue\n");
	TAILQ_INIT(&running_queue);
	TAILQ_INIT(&blocking_queue);
	TAILQ_INIT(&zombie_queue);
	//printf("Spawing first process\n");
	//print_stack_pointer("Stack Init");
	if(!setjmp(spawn_hack_2)){
	  printf("setting up spawn hack");
	  ult_spawn_hack();
	}
	
	ult_spawn(f);
	struct tailque_entry *tcb_process= TAILQ_FIRST(&running_queue);
	if(setjmp(init)){
		schedule();
	}
	else{
		longjmp(tcb_process->context.context,1);
	}
}
/*checks whether tid is needed by a process in blocking queue, if yes, removes item and places it in front of running queue
*/
void is_needed_by_process(int tid){
	struct tailque_entry *item;
	struct tailque_entry *tmp_item;
	for (item = TAILQ_FIRST(&blocking_queue); item != NULL; item = tmp_item){
        tmp_item = TAILQ_NEXT(item, entries);
		int foo = item->context.is_waiting_for;
        if (foo == tid) {
       		TAILQ_REMOVE(&blocking_queue, item, entries);
			TAILQ_INSERT_HEAD(&running_queue,item,entries);
        }
    }

/*Easy round robin scheduler,assumes that rotation has already been done */
}
void schedule(){
	//printf("I'm in the scheduler\n");
	//print_stack_pointer("Stack scheduler");
	//print_queue();
	//TODO: implement error handling if running_queue is empty and blocked queue is not empty
	if(!TAILQ_EMPTY(&running_queue)){
	  struct tailque_entry *thread=TAILQ_FIRST(&running_queue);
	  global_funct_ptr=thread->context.function;
	  longjmp(thread->context.context,1);
	}
	
}
/*debug function*/
void print_queue(){
	struct tailque_entry *pid;
	printf("Running Queue:\n");
	TAILQ_FOREACH(pid,&running_queue,entries){
		printf("%d \n", pid->context.id );
	}
	printf("Blocking Queue:\n");
	TAILQ_FOREACH(pid,&blocking_queue,entries){
		printf("%d %d \n", pid->context.id,pid->context.is_waiting_for );
	}
	printf("Zombie Queue:\n");
	TAILQ_FOREACH(pid,&zombie_queue,entries){
		printf("%d \n", pid->context.id );
	}
}

/*void print_stack_pointer( char *info )
{
  long stackp = 0;
  long basep = 0;

  __asm__ __volatile__ ( "mov %%ebp, %%eax":"=a" (basep));
  __asm__ __volatile__ ( "mov %%esp, %%eax":"=a" (stackp));
  
  printf("%s Stackpointer : %lu Basepointer: %lu\n",info,stackp,basep);

  return;
}*/
