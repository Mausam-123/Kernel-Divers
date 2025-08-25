#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>

#define PAGE_SIZE (4096)

int main() 
{
	int fd;
	char c;
	size_t ret = 0;
	char text[4096] = {};
	void *ptr = NULL;

	fd = open("/dev/my_char_dev", O_RDWR);

	if (fd < 0) {
		perror("open");
		return fd;
	}

	ret = write(fd, "Hello Mausam", sizeof("Hello Mausam")); 
	if (ret == 0) {
		printf("No data written\n");
	} else {
		printf("written %lu bytes\n", ret);
	}

	ptr = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (ptr == NULL) {
		printf("mmap() failed\n");
	} else {
		printf("mmap() passed\n");
		memcpy(text, ptr, PAGE_SIZE);
		printf("Got '%s'\n", (char *)ptr);
		munmap(ptr, PAGE_SIZE);
	}
	
	close(fd);
	return 0;
}
