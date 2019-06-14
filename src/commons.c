#include "commons.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define CHUNK_SIZE 1024 

char* readn(int fd) {
	char chonk[CHUNK_SIZE + 1];
	char c;
	size_t chonk_pos = 0;
	char* msg =(char*) malloc(sizeof(char));
	msg[0] = '\0';

	size_t total_read = 0, read_now = 0;
	while((read_now = read(fd, &c, 1)) > 0) {
		total_read += 1;
		chonk[chonk_pos] = c;
		chonk_pos ++;	

		if(chonk_pos == CHUNK_SIZE || c == '\n') {
			chonk[chonk_pos] = '\0';
			chonk_pos = 0;
			msg = (char*)realloc(msg, sizeof(char) * total_read + 1);
			strcat(msg, chonk);
		}
		if(c == '\n') {
			break;
		}
	}
	return msg;
}

int writen (int fd, const char* buf, size_t buflen) {
	if(buf != NULL) {
		size_t total_written = 0;
		while(buflen > 0) {
			size_t written_now = 0;
			SC(written_now = write(fd, buf, buflen ));
			buflen -= written_now;
			total_written += written_now;
		}
		return total_written;
	}
	return 0;
}
