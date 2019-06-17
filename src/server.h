#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>

struct server_info_s {
	ssize_t active_clients;
	int server_running;
	int server_fd;
};

void create_worker_for_fd (int client_fd);
void server_print_info (const struct server_info_s* info);

#endif
