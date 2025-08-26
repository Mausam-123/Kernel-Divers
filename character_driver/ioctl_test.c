#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/ioctl.h>
#include "ioctl_control.h"

int main() {
	int fd = 0;
	int write_data = 100;
	int read_data = 0;
	struct user_metadata user_data;
	int ioctl_status = 0;	

	strcpy(user_data.name, "Hello Kernel Developers");
	user_data.count = 2;	// Number of times to repeat

	fd = open("/dev/my_char_dev", O_RDWR);
	if (fd < 0) {
		printf("Unable to open device node\n");
		return -1;
	}

	ioctl_status = ioctl(fd, IOCTL_WR_DATA, &write_data);
	if (ioctl_status) {
		printf("Write data failed\n");
		return -1;
	}

	ioctl_status = ioctl(fd, IOCTL_RD_DATA, &read_data);
	if(ioctl_status) {
		printf("Read data failed\n");
		return -1;
	}
	printf("Read %d from kernel space\n", read_data);

	ioctl_status = ioctl(fd, IOCTL_WR_METADATA, &user_data);
	if(ioctl_status) {
		printf("Write metadata failed\n");
		return -1;
	} 

	
	return 0;
}
	
