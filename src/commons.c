#include "commons.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define CHUNK_SIZE 1024 

char* readn(int fd) {
	char chonk[CHUNK_SIZE];
	char* msg =(char*) malloc(sizeof(char));
	msg[0] = '\0';

	size_t total_read = 0, read_now = 0;
	while((read_now = read(fd, chonk, CHUNK_SIZE)) > 0) {
		total_read += read_now;
		msg = (char*)realloc(msg, sizeof(char) * total_read + 1);
		strcat(msg, chonk);
	}
	return msg;
}

int writen (int fd, const char* buf, size_t buflen) {
	if(buf != NULL) {
		size_t total_written = 0;
		while(buflen > 0) {
			size_t written_now = 0;
			SC(written_now = write(fd, buf, buflen));
			buflen -= written_now;
			total_written += written_now;
		}
		return total_written;
	}
	return 0;
}
