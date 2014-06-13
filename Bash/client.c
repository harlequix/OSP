#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <sys/types.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define PORT 9000
#define HOST "127.0.0.1"

int main()
{
	int sockid;
	char buffer[32768];
	struct sockaddr_in addr;
	
	//socket erstellen
    sockid = socket(AF_INET, SOCK_STREAM, 0);

	//zum Server verbinden
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	inet_pton(AF_LOCAL, HOST, &addr.sin_addr);
    connect(sockid,(struct sockaddr *) &addr,sizeof(addr));
	
	//beliebig viele Eingaben
	while (1) {
		printf("Bitte Kommando eingeben: ");
		bzero(buffer,32768);
		fgets(buffer,32767,stdin);
		
		//stoppe bei Eingabe von exit
		if (strcmp("exit\n", buffer)==0) {
			//sage Server, dass er aufhören kann zu warten
			write(sockid,buffer,strlen(buffer));
			break;
		}
		if(strncmp("put",buffer,3)==0){
		  //TODO: remove unecessary strcpy
		  buffer[strlen ( buffer ) - 1] = ' ';
		  int i;
		  char buffer_tmp[strlen(buffer)+1];
		  strcpy(buffer_tmp,buffer);
		  i=0;
		  char *geteilt;
		  char *argv[20];
		  geteilt = strtok(buffer_tmp, " ");
		  while(geteilt != NULL) {
			argv[i]=geteilt;
			geteilt = strtok(NULL, " ");
			i++;
		  }
		  argv[i]=0;
		   int f= open(argv[1],O_RDONLY);
		  if(!f){
		   printf("An error has occured\n");
		   bzero(buffer,32768);
		  }
		  else{
		    //TODO: Fix buffer underflow
		    char foo[32767];
		    read(f,foo, 32767);
		    strncat(buffer,foo,2000);
		    write(sockid,buffer,strlen(buffer));
		    bzero(buffer,32768);
		    //lesen und ausgeben von Antwort
		    read(sockid,buffer,32767);
		    printf("%s\n",buffer);
		    close(f);
		  }	
		}
		
		if(strncmp("get", buffer ,3)==0){
		  int i;
		  char buffer_tmp[strlen(buffer)+1];
		  strcpy(buffer_tmp,buffer);
		  i=0;
		  char *geteilt;
		  char *argv[20];
		  geteilt = strtok(buffer_tmp, " ");
		  while(geteilt != NULL) {
			argv[i]=geteilt;
			geteilt = strtok(NULL, " ");
			i++;
		  }
		  argv[i]=0;
		  write(sockid,buffer,strlen(buffer));
		  bzero(buffer,32768);
		  read(sockid,buffer,32767);
		  if(!strncmp("-1", buffer, 2)){
		    printf("An error has occured\n");
		  }
		  else{
		    argv[1][strlen ( argv[1] ) - 1] = '\0';
		    int f= open(argv[1],O_WRONLY | O_CREAT | O_APPEND, 00644);
		    write(f,buffer,strlen(buffer));
		    close(f);
		  }	
		}
		  
		
		else {
			//an Server schreiben
			write(sockid,buffer,strlen(buffer));
			bzero(buffer,32768);
			
			//lesen und ausgeben von Antwort
			read(sockid,buffer,32767);
			printf("%s\n",buffer);	
		}

	}
	
	//socket schließen
	close(sockid);
	return EXIT_SUCCESS;
}
