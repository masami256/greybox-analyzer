#include <stdio.h>
#include <string.h>

void greeting(char *arg)
{
	printf("arg: %s\n", arg);
	if (!strncmp(arg, "crash", strlen("crash"))) {
		char *p = NULL;
		p[1] = 0x41;
	}
}


