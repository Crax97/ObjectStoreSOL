#include "worker.h"
#include "commons.h"
#include "commands.h"
#include "server.h"

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <memory.h>
#include <poll.h>
#include <unistd.h>
#include <limits.h>


pthread_mutex_t list_mutex = PTHREAD_MUTEX_INITIALIZER;
worker_list active_workers;

struct worker_s* create_worker(int file_descriptor, struct server_info_s *server) {
	struct container_s* container = (struct container_s*) malloc(sizeof(struct container_s));
	struct worker_s* worker = (struct worker_s*) malloc(sizeof(struct worker_s));
	container->server = server;
	container->worker = worker;
	worker->worker_fd = file_descriptor;
	worker->next = NULL;
	worker->prev = NULL;
	worker->is_active = OS_TRUE;
	worker->is_registered = OS_FALSE;
	memset(worker->associated_name, 0, MAX_CLIENT_NAME_LEN + 1);
	pthread_t worker_thread;
	SC(pthread_create(&worker_thread, NULL, worker_cycle, container)); 
	SC(pthread_detach(worker_thread));
	return worker;
}

int handle_msg(char* msg, struct worker_s* worker, struct server_info_s* info) {
	if(msg == NULL) {
		return OS_FALSE;
	}

	char* last = NULL;
	char* command = strtok_r(msg, " \n", &last);

	if(command == NULL) {
		return OS_FALSE;
	}

	printf(OS "Client %s sent %s\n", worker->is_registered ? worker->associated_name : "[No name yet]", command);

	if(strcmp(command, "REGISTER") == 0) {
		char* requested_name = strtok_r(NULL, " \n", &last);
		return handle_register(requested_name, worker, info);
	} else if(strcmp(command, "STORE") == 0) {
		char* data_name = strtok_r(NULL, " \n", &last);
		char* data_length_str = strtok_r(NULL, " \n", &last);
		return handle_store(data_name, data_length_str, worker);
	} else if(strcmp(command, "RETRIEVE") == 0) { 
		char* data_name = strtok_r(NULL, " \n", &last);
		return handle_retrieve(data_name, worker);
	} else if(strcmp(command, "DELETE") == 0) {
		char* data_name = strtok_r(NULL, " \n", &last);
		return handle_delete(data_name, worker);
	} else if(strcmp(command, "LEAVE") == 0) {
		return handle_leave(worker, info);
	} else {
		return ko("Unrecognised command!", worker);
	}
}

void add_worker_list(struct worker_s* worker) {
	pthread_mutex_lock(&list_mutex);

	worker->next = active_workers;
	if(active_workers != NULL) {
		active_workers->prev = worker;
	}
	active_workers = worker;

	pthread_mutex_unlock(&list_mutex);
}

void remove_worker_list(struct worker_s* worker) {
	pthread_mutex_lock(&list_mutex);
	
	if(worker->next != NULL) {
		worker->next->prev = worker->prev;
	}

	if(worker->prev != NULL) {
		worker->prev->next = worker->next;
	}

	if(active_workers == worker) {
		active_workers = worker->next;
	}
	pthread_mutex_unlock(&list_mutex);
}

int is_worker_name_unique(const char* name) {
	pthread_mutex_lock(&list_mutex);
	int is_unique = OS_TRUE;
	struct worker_s* cur_worker = active_workers;
	while(cur_worker != NULL && is_unique) {
		if (strcmp(name, cur_worker->associated_name) == 0) {
			is_unique = OS_FALSE; 
		}	
		cur_worker = cur_worker->next;
	}
	pthread_mutex_unlock(&list_mutex);
	return is_unique;
}

void accept_worker(const char* name, struct worker_s* worker, struct server_info_s* server) {
	pthread_mutex_lock(&list_mutex);
	strcpy(worker->associated_name, name);
	worker->is_registered = OS_TRUE;
	server->active_clients ++;

	char path[PATH_MAX];
	sprintf(path, "%s/%s", DATA_FOLDER, name);
	SC(create_folder(path));
	pthread_mutex_unlock(&list_mutex);
}

void stop_worker(struct worker_s* worker, struct server_info_s *server) {
	pthread_mutex_lock(&list_mutex);
	worker->is_active = OS_FALSE;
	worker->is_registered = OS_FALSE;
	server->active_clients --;

	pthread_mutex_unlock(&list_mutex);
}

void* worker_cycle(void* args) {
	struct container_s* container = (struct container_s*) args;

	struct worker_s* my_info = container->worker;
	struct server_info_s* server = container->server;
	struct pollfd fd;
	fd.fd = my_info->worker_fd;
	fd.events = POLLIN;

	while(server->server_running && my_info->is_active) {
		if(poll(&fd, 1, 10) > 0) {
			char* incoming_msg = NULL;
			int result = read_to_newline(my_info->worker_fd, &incoming_msg);
			if(result <= 0) {
				fprintf(stderr, OS "Client %s (fd %d) quit\n", my_info->associated_name, my_info->worker_fd);
				stop_worker(my_info, server);
				break;
			}
			handle_msg(incoming_msg, my_info, server);
            free(incoming_msg);
		}
	}

	remove_worker_list(my_info);	
	SC(close(my_info->worker_fd));
	free(my_info);
	free(container);
	
	pthread_exit(NULL);
}
