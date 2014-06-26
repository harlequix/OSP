#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tcb.h"

/*speichert den aktuellen Prozessor-Kontext in der übergebenen 
 Kontext-Datenstruktur (wird der gespeicherte Kontext später wieder 
 in den Prozessor geladen, so wird die Abarbeitung hinter tcb_getcontext() fortgesetzt)*/
void tcb_getcontext(tcb *t) {
  setjmp(t->context);
}

/*lädt den Kontext aus der übergebenen Datenstruktur in den Prozessor
 (Befehle nach dieser Funktion werden nicht ausgeführt)*/
void tcb_setcontext(const tcb *t) {
}

/*speichert den Prozessor-Kontext in der ersten Datenstruktur ourTcb und lädt den Kontext
 aus der zweiten Datenstruktur newTcb in den Prozessor (beim erneuten Laden des gespeicherten
 Kontextes taucht das Programm wieder aus dieser Funktion auf)*/
void tcb_swapcontext(tcb *ourTcb, tcb *newTcb) {
  if(setjmp(ourTcb->context)){
    return;
  }
  else{
    tcb_setcontext(newTcb);
  }
}

/*erzeugt aus einer als Funktionszeiger übergebenen Funktion einen neuen Kontext;
 wird dieser Kontext später geladen (mit tcb_setcontext() oder tcb_swapcontext()),
 so beginnt die Abarbeitung der Funktion.*/
void tcb_makecontext(tcb *t, void *func(), int argc, char *argv[]) {
}