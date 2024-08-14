#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern void greeting(char *arg);

void print_hello(void)
{
	puts("Hello, World!\n");
}

int main(int argc, char **argv)
{
	char buf[64] = { 0 };

	printf("Input: ");
	fflush(stdout);
	read(fileno(stdin), buf, sizeof(buf) - 1);

	if (!strcmp(buf, "hello"))
		print_hello();

	void (*f)(char *) = greeting;
	f(buf);

	return 0;
}

