#include <stdio.h>
#include <stdlib.h>
#include <linux/unistd.h>
#include <linux/sysctl.h>
#include <linux/version.h>
#include <errno.h>
#include <string.h>


int main(int argc, char **argv)
{
	int name[] = {CTL_KERN, KERN_NODENAME};
	int namelen = 2;
	char buffer[16];
	int len = sizeof(buffer);
	int error;
	/* defining structs containing arguments for syscall */
	struct __sysctl_args args = {
		name, namelen,
		buffer, &len, /* old */
		argv[1], 0,  /* new */
		};
	/* filling struct, depending on the number of arguments */
	if (argc == 1) {
		args.newval = NULL; /* don't change */
	} else if (argc == 2) {
		args.oldval = NULL; /* don't read */
		
		args.newlen = strlen(argv[1]);
	} else {
		fprintf(stderr,"%s: Specify 0 or 1 argument\n",argv[0]);
		exit(1);
	}
	/* executing syscall and error evaluation */
	error = sysctl(args);
	if (error) {
		fprintf(stderr,"%s: sysctl(): %s\n",
			argv[0],strerror(errno));
		exit(1);
	}

	if (argc == 1) {
		buffer[len+1] = '\0';
		printf("%s\n",buffer);
	}

	exit(0);
}
