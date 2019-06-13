#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "server.h"
#include "commons.h"

#define MAX_CLIENT_NAME_LEN 100

pthread_mutex_t server_info_mutex = PTHREAD_MUTEX_INITIALIZER;
int SERVER_RUNNING = 1;

struct client_info_s {
	struct client_info_s* prec;
	pthread_t client_thread;
	int client_fd;
	char* client_name;
	struct client_info_s* next;
};

struct server_info_s {
	struct client_info_s* clients;
	struct client_info_s* clients_head;
	size_t clients_connected;
};

void* thread_worker(void* args);
void* signal_thread(void* args);

int main(int argc, char** argv) {
	int socket_fd = 0;
	struct sockaddr_un sock;	
	sock.sun_family = AF_UNIX;
	strcpy(sock.sun_path, SOCKNAME);			
	SC(socket_fd = socket(AF_UNIX, SOCK_STREAM, 0));
	SC(bind(socket_fd, (struct sockaddr*)&sock, sizeof(sock)));
	SC(listen(socket_fd, BACKLOG));

	struct server_info_s server;
	server.clients = NULL;
	server.clients_head = NULL;
	server.clients_connected = 0;

	while(1) {
		int client_fd = 0;
		socklen_t len;
		SC(client_fd = accept(socket_fd, (struct sockaddr*)&sock, &len));
		
		create_worker_for_fd( &server, client_fd );
	}

}

void create_worker_for_fd(struct server_info_s *info, int client_fd) {
	struct client_info_s* client = (struct client_info_s*)  malloc(sizeof(struct client_info_s));
	pthread_mutex_lock(&server_info_mutex);
	client->client_fd = client_fd;
	client->next = NULL;
	client->prec = info->clients_head;
	info->clients_head->next = client;	
	pthread_mutex_unlock(&server_info_mutex);

	pthread_t worker_thread;
	SC(pthread_create(&worker_thread, NULL, thread_worker, &client)); 
	client->client_thread = worker_thread;
	printf("[ Object Store] Got a new Client: %d\n", client_fd);
}

void* signal_thread(void* args) {
	sigset_t set;
	sigfillset(&set);
	pthread_sigmask(SIG_SETMASK, &set, NULL);	
}

void* thread_worker(void* args) {
	struct client_info_s* my_info = (struct client_info_s*)args;
	while(SERVER_RUNNING) {
		char* msg = readn(my_info->client_fd);
		char* cmd = strtok(msg, " ");
		// Check CMD	
		printf("Received %s\n", cmd);

		free(msg);
	}	
	pthread_exit(NULL);
}
