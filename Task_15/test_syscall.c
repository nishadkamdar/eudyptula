#include <unistd.h>
#include <stdio.h>

int main (int argc, char *argv)
{
	long error = 0;

	error = syscall(334, 0, 1234567890);
	printf("The error obtained is %d\n", error);

	return 0;
}
