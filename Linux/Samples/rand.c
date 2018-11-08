#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>

void main() {

	int fd = open("thoughts.txt", O_RDONLY);
	int len = lseek(fd, 0, SEEK_END);
	char *data = (char *)mmap(0, len, PROT_READ, MAP_PRIVATE, fd, 0);
	printf("%s\n", data);
	int i;
	for (i = 0; data[i] != '\0'; i++)
		printf("%c\n",(char) data[i]);
}
