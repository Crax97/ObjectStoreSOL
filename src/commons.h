#ifndef COMMONS_H
#define COMMONS_H

#include <stdio.h>

#define WORKING_DIR_NAME "data"
#define SOCKNAME "objstore.sock"
#define BACKLOG 1000
#define SC(c) if((c) < 0) { perror(#c); exit(-1); }

#define MAX_LINE_LENGTH 1024

char* read_to_newline(int fd);
char* read_data(int fd, size_t len);
int writen(int fd, const char* buf, size_t len);

#endif
