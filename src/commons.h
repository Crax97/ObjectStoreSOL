#ifndef COMMONS_H
#define COMMONS_H

#include <stdio.h>

#define WORKING_DIR_NAME "data"
#define SOCKNAME "objstore.sock"
#define BACKLOG 1000
#define SC(c) if((c) < 0) { perror(#c); exit(-1); }

#define MAX_LINE_LENGTH 1024

#define OS_OK 1
#define OS_ERR 0 
#define OS_NOCONN -2

#define REGISTER_STR "REGISTER %s \n"
#define STORE_STR "STORE %s %lu \n "
#define OK_STR "OK \n"
#define RETRIEVE_STR "RETRIEVE %s \n"
#define DATA_STR "DATA %lu \n "
#define DELETE_STR "DELETE %s \n"
#define LEAVE_STR "LEAVE \n"

char* read_to_newline(int fd);
char* read_data(int fd, size_t len);
int writen(int fd, const char* buf, size_t len);

#endif
