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
//Deklaration der Queues
TAILQ_HEAD(,tailque_entry) running_queue;
TAILQ_HEAD(,tailque_entry) blocking_queue;
TAILQ_HEAD(,tailque_entry) zombie_queue;
jmp_buf init;
void is_needed_by_process(int id);

/*
 This function only exists to tell the process to use an empty stack for the thread
 */
jmp_buf spawn_buf;
ult_func global_funct_ptr;

void signalHandlerSpawn( ){	
	ult_func handler_func_ptr= global_funct_ptr;
	if(setjmp(spawn_buf)){
		handler_func_ptr();
		//printf("Jump successful!\n");
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
	tcb *spawn;
  	stack_t stack;
  	struct sigaction sa;
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
	
  	printf("Altering Signal\n");
	
  	sa.sa_flags = SA_ONSTACK;
  	sa.sa_handler=&signalHandlerSpawn;
  	sigemptyset( &sa.sa_mask );
  	sigaction( SIGUSR1, &sa, 0 );
	
  	printf("raising Signal\n");
  	global_funct_ptr=f;
	
  	raise( SIGUSR1 );
	
  	printf("back from signalhandler\n");
  	printf("Saving tcb and pushing into the queue\n");
	
  	*spawn->context=*spawn_buf;
  	spawn->id=id;
  	spawn->status=0;
  	spawn->stack=stack;
	struct tailque_entry *item;
	item = (struct tailque_entry*) malloc(sizeof(struct tailque_entry));
	item->context=*spawn;
	TAILQ_INSERT_TAIL(&running_queue, item, entries);
	printf("checking if process is in queue\n");
	//TAILQ_FOREACH(item, &running_queue, entries){
	//	printf("TID: %d\n", item->context.id);
	//}
	
	
	return id;		
}


// yield the CPU to another thread
/*	Die Funktion ult_yield() gibt innerhalb eines Threads freiwillig den
 Prozessor auf. Es erfolgt der Aufruf des Schedulers und die Umschaltung
 auf den vom Scheduler ausgewaehlten Thread.
 */
void ult_yield() {
	struct tailque_entry *tcb = TAILQ_FIRST(&running_queue);
	
	//TODO: check for identical process hereafter (or if running_queue only continues only on )
	TAILQ_INSERT_TAIL(&running_queue, tcb, entries);
	TAILQ_REMOVE(&running_queue, tcb, entries);
	printf("FOOOOOOBAR\n");
	longjmp(init,1);
}



// current thread exits
/*	Wird innerhalb eines Threads ult_exit() aufgerufen, so wird der Thread
 zum Zombie und der Exit-Status wird abgespeichert.
 */
void ult_exit(int status) {
	struct tailque_entry *tcb = TAILQ_FIRST(&running_queue);
	tcb->context.status=status;
	is_needed_by_process(tcb->context.id);
	TAILQ_INSERT_TAIL(&zombie_queue, tcb, entries);
	TAILQ_REMOVE(&running_queue, tcb, entries);
	printf("FOOOOOOBAR\n");
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
	setjmp(blocked->context.context);
	TAILQ_FOREACH(pid,&zombie_queue,entries){
		if(pid->context.id==tid){
			*status=pid->context.status;
			return 0;
		}
	}
	blocked->context.is_waiting_for=tid;
	TAILQ_INSERT_TAIL(&blocking_queue, blocked, entries);
	TAILQ_REMOVE(&running_queue, blocked, entries);
	longjmp(init,1);
	

	return -1;	//return 'error'
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
	return 0;
}


// start scheduling and spawn a thread running function f
/*	Die Funktion ult_init() initialisiert die Bibliothek (insbesondere die Datenstrukturen
 des Schedulers wie z.B. Thread-Listen) und muss vor allen anderen Funktionen aufgerufen
 werden. Sie erzeugt einen einzigen Thread (analog zum Init-Thread‰ bei Unix), aus
 welchem heraus dann alle anderen Threads erzeugt werden und welcher danach mit
 ult_waitpid() auf das Ende aller Threads wartet. 
 */
/*struct tailque_entry{
 tcb context;
 TAILQ_ENTRY(tailque_entry) entries;
 //TODO: eventually unifying tcb and tailque_entry
 };
 TAILQ_HEAD(,tailque_entry) running_queue;
 TAILQ_HEAD(,tailque_entry) blocking_queue;
 TAILQ_HEAD(,tailque_entry) zombie_queue;
 */
int jmpinfo;
void ult_init(ult_func f) {
	
	printf("Setting up queue\n");
	TAILQ_INIT(&running_queue);
	TAILQ_INIT(&blocking_queue);
	TAILQ_INIT(&zombie_queue);
	printf("Spawing first process\n");
	
	ult_spawn(f);
	struct tailque_entry *tcb_process= TAILQ_FIRST(&running_queue);
	if(setjmp(init)){
		printf("I'm back %d \n",TAILQ_EMPTY(&running_queue));
	}
	else{
		longjmp(tcb_process->context.context,1);
	}
	
	
	
	
}
void is_needed_by_process(int tid){
	struct tailque_entry *pid;
	TAILQ_FOREACH(pid,&blocking_queue,entries){
		if(pid->context.id==tid){
			TAILQ_INSERT_HEAD(&running_queue,pid,entries);
			TAILQ_REMOVE(&zombie_queue,pid,entries);
			return;
		}
	}
}
