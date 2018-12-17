#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

int main(void) {
	size_t d =107374182400;
	printf("%zd\n", d);
FILE *inp=fopen("input.txt","r");
fseek(inp,0, SEEK_END);
size_t fsz = ftell(inp);
fclose(inp);
int fd = open("input.txt", O_RDONLY);
char * region = mmap(0,fsz,PROT_READ, MAP_FILE|MAP_PRIVATE,fd,0);
int unmap_result = munmap(region, fsz);
close(fd);
return 0;
}