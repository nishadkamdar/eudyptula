#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	unsigned long id;
	unsigned long set_ver = 0xabcd1234;
	int fd;
	int ret;

	if (argc != 2) {
		printf("Usage: %s FILENAME\n", argv[0]);
		exit (EXIT_FAILURE);
	}

	fd = open(argv[1], O_RDONLY);
	if (fd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}


	ret = ioctl(fd, EXT2_IOC_GETVERSION, &id);
	if (ret == -1) {
		perror("ioctl");
		exit(EXIT_FAILURE);
	}
	printf("inode version is %x\n", id);

	/*printf("Setting Volume ID to 0xabcd1234\n");
	ret = ioctl(fd, FAT_IOCTL_SET_VOLUME_ID, &set_id);
	if (ret == -1) {
		perror("ioctl");
		exit(EXIT_FAILURE);
	}
	
	ret = ioctl(fd, FAT_IOCTL_GET_VOLUME_ID, &id);
	if (ret == -1) {
		perror("ioctl");
		exit(EXIT_FAILURE);
	}
	
	printf("Volume ID %4x-%4x\n", id >> 16, id & 0xFFFF);*/

	close(fd);

	exit(EXIT_SUCCESS);
}
