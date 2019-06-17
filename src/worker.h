#ifndef WORKER_H
#define WORKER_H

#include <pthread.h>
#define MAX_CLIENT_NAME_LEN 100

struct worker_s {
	struct worker_s *next;
	pthread_t worker_thread;
	int worker_fd;
	int is_active;
	int is_registered;
	char associated_name[MAX_CLIENT_NAME_LEN + 1];
	struct worker_s *prev;
};
typedef struct worker_s* worker_list;

struct worker_s* create_worker(int file_descriptor);
void add_worker_list(struct worker_s* worker);
void remove_worker_list(struct worker_s* worker);
int is_worker_name_unique(const char* name);
void accept_worker(const char* name, struct worker_s* worker); 
void stop_worker(struct worker_s* worker);

void* worker_cycle(void* arg);

#endif
