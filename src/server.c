#include "server.h"
#include "commons.h"
#include "worker.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <limits.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>

#define MAX_CLIENT_NAME_LEN 100
#define BUF_SIZE 1024
#define OK_STR "OK \n"

#define DEFAULT_MASK 0777

struct client_info_s {
	struct client_info_s* prec;
	pthread_t client_thread;
	int client_fd;
	int has_registered;
	int is_connected;
	char client_name[MAX_CLIENT_NAME_LEN + 1];
	struct client_info_s* next;
};

struct server_info_s {
	int fd;
	pthread_t signal_thread;
	struct client_info_s* clients;
	struct client_info_s* clients_head;
	int clients_connected;
};

pthread_mutex_t server_info_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t server_disconnect = PTHREAD_COND_INITIALIZER;
struct server_info_s server;
volatile int SERVER_RUNNING = 1;

void add_to_active_clients(struct client_info_s* client);
void remove_from_active_clients(struct client_info_s* client);
void sigusr_handler_f(int sig);
void close_everyone(void);

void* thread_worker(void* args);
void* signal_thread_worker(void* args);
int send_ok(int fd);
int send_ko(int fd, const char* msg);
int handle_cmd(char* msg, struct client_info_s *client);
int check_client_name_unique(char* name);

// Server stuff
int register_client_if_unique(struct client_info_s *client, char* name);
int store_data(struct client_info_s *client, char* data_name, size_t data_len, char* data);
int retrieve_data(struct client_info_s *client, char* data_name);
int delete_data(struct client_info_s *client, char* data_name);

int main(int argc, char** argv) {
	unlink (SOCKNAME); // Just in case
	struct sockaddr_un sock;	
	sock.sun_family = AF_UNIX;
	strcpy(sock.sun_path, SOCKNAME);			

	int socket_fd = 0;
	SC(socket_fd = socket(AF_UNIX, SOCK_STREAM, 0));
	SC(bind(socket_fd, (struct sockaddr*)&sock, sizeof(sock)));
	SC(listen(socket_fd, BACKLOG));

	server.clients = NULL;
	server.clients_head = NULL;
	server.clients_connected = 0;
	server.fd = socket_fd;

	mkdir("data", DEFAULT_MASK);

	//sigset_t signals;
	//sigfillset(&signals);
	//pthread_sigmask(SIG_SETMASK, &signals, NULL);

	//SC(pthread_create(&server.signal_thread, NULL, signal_thread_worker, NULL));

	while(SERVER_RUNNING) {
		int client_fd = 0;
		socklen_t len;
		client_fd = accept(socket_fd, (struct sockaddr*)&sock, &len);
		
		if(client_fd > 0) {
			printf(OS "Got a new client on fd %d\n", client_fd);
			struct worker_s* new_worker = create_worker(client_fd);
			add_worker_list(new_worker);
		}
	}

	//SC(pthread_join(server.signal_thread, NULL));
	//close_everyone();
	//while(server.clients_connected > 0 ) {
	//	sched_yield();	
	//}

	unlink(SOCKNAME);
	return EXIT_SUCCESS;
}

void* signal_thread_worker(void* args) {
	sigset_t set;
	sigaddset(&set, SIGUSR1);
	sigaddset(&set, SIGINT);
	pthread_sigmask(SIG_UNBLOCK, &set, NULL);	

	while(SERVER_RUNNING) {
		int signal;
		sigwait(&set, &signal);

		switch(signal) {
			case SIGUSR1:
				printf("Got SIGUSR1\n");
				sigusr_handler_f(signal);

				sigaddset(&set, SIGUSR1);
				sigaddset(&set, SIGINT);
				pthread_sigmask(SIG_UNBLOCK, &set, NULL);

				break;
			case SIGINT:
			case SIGTERM:
				SERVER_RUNNING = 0;
				close(server.fd);
				printf("[Object Store] Initiating server shutdown...\n");
				break;
			
		}

	}

	pthread_exit(NULL);
}
void sigusr_handler_f(int sig) {
	server_print_info(&server);
}

void gather_directory_elts_and_size(DIR* dir, size_t* elts, size_t* size) {
	char buf[PATH_MAX];
	getcwd(buf, PATH_MAX);
	printf("scanning dir %s\n", buf); 
	struct dirent* cur_dir = readdir(dir);
	struct stat info;
	if(stat(cur_dir->d_name, &info) == 0) {
		while(cur_dir != NULL) {
			if (strcmp(cur_dir->d_name, ".") == 0 ||
				strcmp(cur_dir->d_name, "..") == 0) {
				//nothing	
			} else {
				struct stat cur_scanned_info;
				char cur_el[PATH_MAX];
				sprintf(cur_el, "%s/%s", buf, cur_dir->d_name);
				if( stat(cur_el, &cur_scanned_info) == 0 && S_ISDIR(cur_scanned_info.st_mode)) {
					DIR* newdir = opendir(cur_dir->d_name);
					if(newdir != NULL) {
						SC(chdir(cur_dir->d_name));
						gather_directory_elts_and_size(newdir, elts, size);
						closedir(newdir);
					}
				} else {
					(*elts)++;
					(*size) += info.st_size;
				}
			}
			cur_dir = readdir(dir);
		}
	} else {
		perror("Could not open directory");
	}
	SC(chdir(".."));

}	

void server_print_info(const struct server_info_s* info) {
	printf("[Object Store] Server infos:\n");
	size_t total_dim = 0, total_count = 0;
	DIR* data_dir = opendir("data");
	if(data_dir != NULL) {
		chdir("data");
		gather_directory_elts_and_size(data_dir, &total_count, &total_dim);
		closedir(data_dir);
		printf("[Object Store] Clients connected: %d\n", info->clients_connected);
		printf("[Object Store] Total size: %lu\n", total_dim);
		printf("[Object Store] Total elements: %lu\n", total_count);	
	} else {
		fprintf(stderr, "[Object Store] could not open the data directory!");
		exit(EXIT_FAILURE);
	}
}
