#include <fcntl.h>
#include <linux/msdos_fs.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define IOC_MAGIC 'r'
#define FAT_IOCTL_SET_DIR_VOLUME_LABEL _IOW(IOC_MAGIC, 0x15, char *)

int main(int argc, char *argv[])
{
	uint32_t id;
	uint32_t set_id = 0xabcd1234;
	int fd;
	int ret;
	char *newlabel = "nishad";

	if (argc != 2) {
		printf("Usage: %s FILENAME\n", argv[0]);
		exit (EXIT_FAILURE);
	}

	fd = open(argv[1], O_RDONLY);
	if (fd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}


	ret = ioctl(fd, FAT_IOCTL_GET_VOLUME_ID, &id);
	if (ret == -1) {
		perror("ioctl");
		exit(EXIT_FAILURE);
	}
	printf("Volume ID %4x-%4x\n", id >> 16, id & 0xFFFF);

	/*printf("Setting Volume ID to 0xabcd1234\n");
	ret = ioctl(fd, FAT_IOCTL_SET_VOLUME_ID, &set_id);
	if (ret == -1) {
		perror("ioctl");
		exit(EXIT_FAILURE);
	}*/
	
	if (ioctl(fd, FAT_IOCTL_SET_DIR_VOLUME_LABEL, newlabel))
		printf("set volume failed\n");
	else
		printf("successfully changed label\n");

	
	
	ret = ioctl(fd, FAT_IOCTL_GET_VOLUME_ID, &id);
	if (ret == -1) {
		perror("ioctl");
		exit(EXIT_FAILURE);
	}
	
	printf("Volume ID %4x-%4x\n", id >> 16, id & 0xFFFF);

	close(fd);

	exit(EXIT_SUCCESS);
}
