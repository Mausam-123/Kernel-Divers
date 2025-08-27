#include <stdio.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>

int main() {
	int fd;
	struct pollfd pfd;
	char buf[16];
	int ret;
	int n;

	fd = open("/dev/key_device", O_RDONLY);
	
	if (fd < 0) {
		printf("Failed to open device\n");
		return 1;
	}

	pfd.fd = fd;
	pfd.events = POLLIN;  // Wait for data to be available for reading

	printf("Waiting for event (Alt key press)...\n");

	while (1) {
		ret = poll(&pfd, 1, -1);  // Block indefinitely until event
		
		if (ret > 0) {
			if (pfd.revents & POLLIN) {
				buf[16];
				n = read(fd, buf, sizeof(buf));
 				if (strcmp(buf, "1") == 0) {
					printf("ALT Event detected!\n");
					break;
				} else {
					printf("ALT Event not detected, buf : %s\n", buf);
				}
            		} else {
				printf("Event pressed not ALT\n");
			}
		} else if (ret == 0) {
			printf("Poll timed out (should not happen with -1 timeout)\n");
		} else {
			printf("poll");
			break;
		}
	}

	close(fd);
	return 0;
}

