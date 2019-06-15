#include "commons.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define CHUNK_SIZE 1024 

char* read_to_newline(int fd) {
	char chonk[CHUNK_SIZE + 1];
	char c = '\0';
	char* msg =(char*) malloc(sizeof(char));
	msg[0] = '\0';

	size_t total_read = 0, chonk_pos = 0;
   	int	read_now = 0;
	while( c != '\n' ) {
		read_now = read(fd, &c, 1);
		if(read_now > 0) {
			total_read += read_now;
			chonk[chonk_pos] = c;
			chonk_pos ++;
			if( chonk_pos == CHUNK_SIZE || c == '\n') {
				chonk[chonk_pos] = '\0';
				msg = (char*)realloc(msg, (total_read + chonk_pos) * sizeof(char));
				chonk_pos = 0;
				strcat(msg, chonk);
			}
		}
	}
	return msg;
}

char* read_data(int fd, size_t len) {
	char* msg = (char*)calloc(len + 1, sizeof(char));
	msg[len] = '\0';

	char useless;
	SC(read(fd, &useless, 1)); // Useless read because data segments have an useless space in front of them

	size_t total_read = 0;
	int read_now = 0;
	while( total_read < len + 1) {

		read_now = read(fd, msg + total_read, len);
		//if(read_now < 0) {
		//	free(msg);
		//	return NULL;
		//}
		total_read += read_now;
	}
	return msg;
}

int writen (int fd, const char* buf, size_t buflen) {
	if(buf != NULL) {
		size_t total_written = 0;
		while(total_written < buflen) {
			size_t written_now = 0;
			if ((written_now = write(fd, buf, buflen)) < 0) {
					return written_now;
			}
			total_written += written_now;
		}
		return total_written;
	}
	return 0;
}
