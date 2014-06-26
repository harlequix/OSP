
default:
	gcc -Wall -Werror  -c -g ult.c
	gcc -Wall -Werror  -c -g tcb.c
	gcc -Wall -Werror  -c -g test_ult.c
	gcc test_ult.o ult.o tcb.o -o test_ult

clean:
	rm -f *.o test_ult

pack:
	tar cvf ../lab-4.tar ../src