#ifndef WORKER_H
#define WORKER_H

#include <pthread.h>
#include <unistd.h>

#define MAX_CLIENT_NAME_LEN 100

struct server_info_s;

struct worker_s {
	struct worker_s *next;
	pthread_t worker_thread;
	int worker_fd;
	int is_active;
	int is_registered;
	char associated_name[MAX_CLIENT_NAME_LEN + 1];
	struct worker_s *prev;
};

struct container_s {
	struct server_info_s* server;
	struct worker_s* worker;
};

typedef struct worker_s* worker_list;

struct worker_s* create_worker(int file_descriptor, struct server_info_s *server);
void add_worker_list(struct worker_s* worker);
void remove_worker_list(struct worker_s* worker);
int is_worker_name_unique(const char* name);
void accept_worker(const char* name, struct worker_s* worker, struct server_info_s *server); 
void stop_worker(struct worker_s* worker, struct server_info_s *server);

void* worker_cycle(void* arg);

#endif
