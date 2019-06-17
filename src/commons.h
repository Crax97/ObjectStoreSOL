#ifndef COMMONS_H
#define COMMONS_H

#include <stdio.h>

struct server_info_s;

#define WORKING_DIR_NAME "data"
#define SOCKNAME "objstore.sock"
#define BACKLOG 1000
#define SC(c) if((c) < 0) { perror(#c); exit(-1); }
#define CATASTROPHIC_FAILURE(msg) {fprintf(stderr, msg); exit(EXIT_FAILURE);}

#define MAX_LINE_LENGTH 1024
#define DATA_FOLDER "data"
#define DEFAULT_MASK 0777

#define OS_TRUE 1
#define OS_FALSE 0

#define OS_OK OS_TRUE
#define OS_ERR OS_FALSE
#define OS_NOCONN OS_FALSE

#define OS "[Object Store] "

#define REGISTER_STR "REGISTER %s \n"
#define STORE_STR "STORE %s %lu \n "
#define OK_STR "OK \n"
#define RETRIEVE_STR "RETRIEVE %s \n"
#define DATA_STR "DATA %lld \n "
#define DELETE_STR "DELETE %s \n"
#define LEAVE_STR "LEAVE \n"

ssize_t read_to_newline(int fd, char **buf);
ssize_t read_data(int fd, char* buf, size_t len);
ssize_t writen(int fd, const char* buf, size_t len);

int create_folder(char* path);
int read_from_disk(char* path, char** buf);
int write_to_disk(char* path, char* buf, ssize_t len);
int delete_file(char* path);

void server_print_info(const struct server_info_s* info);

#endif
