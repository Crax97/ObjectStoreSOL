#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>

struct server_info_s;

void create_worker_for_fd (struct server_info_s* info, int client_fd);
void server_print_info (const struct server_info_s* info);

#endif
