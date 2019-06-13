#include "commons.h"

#include <stdlib.h>
#include <unistd.h>

int readn(int fd, char* buf, size_t buflen) {
	if(buf != NULL) {

		size_t left_bytes = buflen, total_bytes = 0;
		while (left_bytes > 0) {
			size_t read_bytes = 0;
		    SC(read_bytes = read(fd, buf, left_bytes));
			left_bytes -= read_bytes;
			total_bytes += read_bytes;
		}
		return total_bytes;
	}
	return 0;
}

int writen (int fd, const char* buf, size_t len) {
	if (buf != NULL) {
		size_t left_bytes = len, total_bytes = 0;
		while(left_bytes > 0) {
			size_t read_bytes = 0;
			SC(read_bytes = write(fd, buf, left_bytes));
			left_bytes -= read_bytes;
			total_bytes += read_bytes;
		}
		return total_bytes;
	}
	return 0;
}
