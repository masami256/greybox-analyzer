#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void greeting(char *arg)
{
	printf("arg: %s\n", arg);
	if (!strcmp(arg, "crash")) {
		char *p = NULL;
		p[1] = 0x41;
	}
}

void print_hello(void)
{
	puts("Hello, World!\n");
}

int main(int argc, char **argv)
{
	if (argc != 2) {
		printf("Usage: %s <string>\n", argv[0]);
		exit(1);
	}

	FILE *fp = fopen("/tmp/prog.txt", "w+");

	fprintf(fp, "input: %s\n", argv[1]);
	fclose(fp);

	if (!strcmp(argv[1], "hello"))
		print_hello();

	greeting(argv[1]);

	return 0;
}

