#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <sys/types.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>

#define PORT 9000

int main()
{	
	int sockid, connection, status, i, savestdout;
	socklen_t clientlen;
	char buffer[32768];
	struct sockaddr_in addr, client;
	int stdoutpipe[2];
	pid_t childPID;
	char *geteilt;
	
	
	//socket erstellen
	sockid = socket(AF_INET, SOCK_STREAM, 0);
	
	//an socket binden
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(PORT);
    bind(sockid, (struct sockaddr *) &addr, sizeof(addr));

	//zuhören an socket
	listen(sockid,5);
	
	//connection annehmen
	clientlen = sizeof(client);
	connection = accept(sockid, (struct sockaddr *) &client, &clientlen);
	while(1) {
		//vom Client lesen
		bzero(buffer,32768);
		read(connection,buffer,32767);
		//wenn exit gelesen beenden
		if (strcmp("exit\n", buffer)==0) {
			break;
		}
		else {
			buffer[strlen(buffer)-1] = 0;
			printf("Kommando: %s \n",buffer);
			char *argv[20];

			//String zerteilen
			i=0;
			geteilt = strtok(buffer, " ");
			while(geteilt != NULL) {
				argv[i]=geteilt;
				geteilt = strtok(NULL, " ");
				i++;
			}
			argv[i]=0;
			printf("%s",argv[0]);
			
			//put, get oder Kommando
			if (strcmp(argv[0],"put")==0){
			  int fileDescriptor=open(argv[1],O_WRONLY | O_CREAT | O_APPEND, 00644 );
			  

			  if (write(fileDescriptor, argv[2], strlen(argv[2])) != strlen(argv[2])){
			      write(2, "There was an error writing to testfile.txt\n", 43);
			      return -1;
			  }
			  write(connection,buffer,strlen(buffer));
			}
			else {
				if (strcmp(argv[0],"get")==0) {
				  int fileDescriptor=open(argv[1], O_RDONLY);
				  size_t size;
				  struct stat st;
				  stat(argv[1], &st);
				  size = st.st_size;
				  read(fileDescriptor ,buffer,size);
				  write(connection,buffer,strlen(buffer));
				}
				else{
					//Pipe erstellen und stdout auf Pipe umlenken
					savestdout = dup(STDOUT_FILENO);
					pipe(stdoutpipe);
					dup2(stdoutpipe[1], STDOUT_FILENO);   
					close(stdoutpipe[1]);
					
					//forken und Kommando aufrufen
					childPID = fork();
					if(childPID == 0) //Kindprozess führt Kommando aus
					{
						execvp(argv[0], argv);
						exit(-1);
					}
					else //Elternprozess wartet
					{
						wait(&status);
					}
					
					//aus Pipe lesen
					read(stdoutpipe[0], buffer, 32767);
					
					//stdout zurücksetzen
					fflush(stdout);
					dup2(savestdout, STDOUT_FILENO);
					
					//an Client schreiben
					write(connection,buffer,strlen(buffer));
					
				}	
					
			}
			
		}

	}
	
	//sockets schließen
	close(connection);
	close(sockid);
	return EXIT_SUCCESS;
	

}
