#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

int main(void)
{
	int fd, result, len;
	char buf[] = "Nishad Kamdar";

	fd = open("/sys/kernel/debug/eudyptula/foo", O_WRONLY);
	if (fd < 0)
	{
		fprintf (stderr, "error opening file %s:%s\n", "/sys/kernel/debug/eudyptula/foo", strerror(errno));
		exit(EXIT_FAILURE);
	}

	len = write(fd, buf, 13);
	if (len == -1)
	{
		fprintf(stderr, "error writing file %s:%s\n", "/sys/kernel/debug/eudyptula/foo", strerror(errno));
		exit(EXIT_FAILURE);
	}

	close(fd);
}

