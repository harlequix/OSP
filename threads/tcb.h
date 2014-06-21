#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>


#define STACKSIZE	64*1024	//approriate stack size

struct tcb_str;	//forward declaration

typedef struct tcb_str {
	//fill this struct with statusinformations
	stack_t	stack;	//stack for local vars
	int status;
	jmp_buf context;
	int id;
	/* rest */
} tcb;

/*speichert den aktuellen Prozessor-Kontext in der übergebenen 
Kontext-Datenstruktur (wird der gespeicherte Kontext später wieder 
in den Prozessor geladen, so wird die Abarbeitung hinter tcb_getcontext() fortgesetzt)*/
extern void tcb_getcontext(tcb *t);

/*lädt den Kontext aus der übergebenen Datenstruktur in den Prozessor
 (Befehle nach dieser Funktion werden nicht ausgeführt)*/
extern void tcb_setcontext(const tcb *t);

/*speichert den Prozessor-Kontext in der ersten Datenstruktur ourTcb und lädt den Kontext
 aus der zweiten Datenstruktur newTcb in den Prozessor (beim erneuten Laden des gespeicherten
 Kontextes taucht das Programm wieder aus dieser Funktion auf)*/
extern void tcb_swapcontext(tcb *ourTcb, tcb *newTcb);

/*erzeugt aus einer als Funktionszeiger übergebenen Funktion einen neuen Kontext;
 wird dieser Kontext später geladen (mit tcb_setcontext() oder tcb_swapcontext()),
 so beginnt die Abarbeitung der Funktion.*/
extern void tcb_makecontext(tcb *t, void *func(), int argc, char* argv[]);


