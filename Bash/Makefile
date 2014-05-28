all: client server

client:
	gcc -Wall -Werror client.c -o client

server:
	gcc -Wall -Werror server.c -o server

clean:
	rm -f *~ *.o *.out client server

pack:
	tar cvf ../lab-3.tar ../src