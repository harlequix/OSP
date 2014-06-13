

/* type of function executed by thread */
typedef void (*ult_func)();

// spawn a new thread, return its ID 
/*	Die Funktion ult_spawn() erzeugt einen Thread-Control-Block (TCB) für
 die Funktion f() und stellt den Thread in die Run-Queue. Sie liefert
 den ID des erzeugten Threads zurück. An dieser Stelle erfolgt kein
 Scheduling, sondern die Abarbeitung wird mit dem laufenden Thread
 fortgesetzt, um die Erzeugung mehrerer Threads zu ermöglichen.
 */
extern int ult_spawn(ult_func f);


// yield the CPU to another thread
/*	Die Funktion ult_yield() gibt innerhalb eines Threads freiwillig den
 Prozessor auf. Es erfolgt der Aufruf des Schedulers und die Umschaltung
 auf den vom Scheduler ausgewählten Thread.
 */
extern void ult_yield(); 


// current thread exits
/*	Wird innerhalb eines Threads ult_exit() aufgerufen, so wird der Thread
 zum Zombie und der Exit-Status wird abgespeichert.
 */
extern void ult_exit(int status); 



// thread waits for termination of another thread
// returns 0 for success, -1 for failure
// exit-status of terminated thread is obtained
/*	Mit ult_waitpid() kann abgefragt, ob der Thread mit dem angegebenen ID
 bereits beendet wurde. Ist der Thread bereits beendet, so kehrt die
 Funktion sofort zurück und liefert in status den Exit-Status des Threads
 (welcher als Argument an ult_exit() übergeben wurde). Läuft der Thread noch,
 so soll der aktuelle Thread blockieren und es muss auf einen anderen Thread
 umgeschaltet werden. Bei nachfolgenden Aufrufen des Schedulers soll überprüft
 werden, ob der Thread mittlerweile beendet wurde; ist dies der Fall, so soll
 der aktuelle Thread geweckt werden (am besten so, dass er auch als nächster 
 Thread die CPU erhält).
 */
extern int ult_waitpid(int tid, int *status);



// read from file, block the thread if no data available
/*	Hinter ult_read() verbirgt sich die Funktion read() der Unix-API, welche Daten
 aus einer Datei (File-Deskriptor fd) in einen Puffer (buf) liest. Die Funktion
 ult_read() ist eine Wrapper-Funktion für read(), welche zusätzlich überprüft, ob
 ausreichend Daten verfügbar sind. Ist dies nicht der Fall, so wird der Thread
 blockiert und ein anderer Thread erhält die CPU. Bei nachfolgenden Aufrufen des
 Schedulers soll überprüft werden, ob mittlerweile ausreichend Daten vorliegen; ist
 dies der Fall, so soll der aktuelle Thread geweckt werden (s.o.). Dies löst ein
 Problem mit Systemrufen, welche im Kernel blockieren können (wie z.B. read()).
 Ohne diesen Mechanismus würde die gesamte User-Level-Thread-Bibliothek, die aus Sicht
 des Kernels ein Prozess ist, blockieren (selbst wenn andere User-Level-Threads
 lauffähig wären). Wir kümmern uns hier nur um die read()-Funktion, obwohl auch andere
 Systemrufe blockieren können. 
 */
extern int ult_read(int fd, void *buf, int count); 



// start scheduling and spawn a thread running function f
/*	Die Funktion ult_init() initialisiert die Bibliothek (insbesondere die Datenstrukturen
 des Schedulers wie z.B. Thread-Listen) und muss vor allen anderen Funktionen aufgerufen
 werden. Sie erzeugt einen einzigen Thread (analog zum “Init-Thread” bei Unix), aus
 welchem heraus dann alle anderen Threads erzeugt werden und welcher danach mit
 ult_waitpid() auf das Ende aller Threads wartet. 
 */
extern void ult_init(ult_func f); 


