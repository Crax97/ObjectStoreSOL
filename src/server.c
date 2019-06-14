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
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>

#include "server.h"
#include "commons.h"

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
	struct client_info_s* clients;
	struct client_info_s* clients_head;
	size_t clients_connected;
};


pthread_mutex_t server_info_mutex = PTHREAD_MUTEX_INITIALIZER;
struct server_info_s server;
int SERVER_RUNNING = 1;

void remove_from_active_clients(struct client_info_s* client);
void sigusr_handler(int sig);

void* thread_worker(void* args);
void* signal_thread_worker(void* args);
int send_ok(int fd);
int send_ko(int fd, const char* msg);
int handle_cmd(char* cmd, char* stuff, struct client_info_s *client);

// Server stuff
int register_client(struct client_info_s *client);
int store_data(struct client_info_s *client, char* data_name, size_t data_len, char* data);
int retrieve_data(struct client_info_s *client, char* data_name);
int delete_data(struct client_info_s *client, char* data_name);
int disconnect_client(struct client_info_s *client);

int main(int argc, char** argv) {
	//TBR
	unlink(SOCKNAME);

	int socket_fd = 0;
	struct sockaddr_un sock;	
	sock.sun_family = AF_UNIX;
	strcpy(sock.sun_path, SOCKNAME);			
	SC(socket_fd = socket(AF_UNIX, SOCK_STREAM, 0));
	SC(bind(socket_fd, (struct sockaddr*)&sock, sizeof(sock)));
	SC(listen(socket_fd, BACKLOG));

	server.clients = NULL;
	server.clients_head = NULL;
	server.clients_connected = 0;

	mkdir("data", DEFAULT_MASK);

	sigset_t signals;
	sigemptyset(&signals);
	pthread_sigmask(SIG_BLOCK, &signals, NULL);

	pthread_t signal_thread;
	pthread_create(&signal_thread, NULL, signal_thread_worker, NULL);

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

	client->has_registered = 0;
	client->is_connected = 1;
	client->client_fd = client_fd;
	client->next = NULL;
	client->prec = info->clients_head;
	info->clients_head = client;	

	pthread_mutex_unlock(&server_info_mutex);

	pthread_t worker_thread;
	SC(pthread_create(&worker_thread, NULL, thread_worker, client)); 
	client->client_thread = worker_thread;
	printf("[ Object Store ] Got a new Client: %d\n", client_fd);
}

void* signal_thread_worker(void* args) {
	sigset_t set;
	sigfillset(&set);
	pthread_sigmask(SIG_SETMASK, &set, NULL);	

	struct sigaction sigpipe_handler = {0};
	sigpipe_handler.sa_handler = SIG_IGN;
	sigpipe_handler.sa_flags = SA_RESTART;
	sigaction(SIGPIPE, &sigpipe_handler, NULL);

}

void* thread_worker(void* args) {
	struct client_info_s* my_info = (struct client_info_s*)args;
	while(SERVER_RUNNING && my_info->is_connected) {
		char* msg = readn(my_info->client_fd);
		char* last = NULL;
		char* cmd = strtok_r(msg, " ", &last);
		char* reminder = strtok_r(NULL, "", &last);
		errno = 0;
		if(msg != NULL) {
			if (cmd != NULL && reminder != NULL) {
				handle_cmd(cmd, reminder, my_info);
			} else {
				printf("[ Object Store ] Client %d sent garbage. Trying to send KO message\n", my_info->client_fd);
				if(send_ko(my_info->client_fd, "Bad request") < 0) {
					printf("[ Object Store ] Client %d probably closed the connection\n", my_info->client_fd);
					remove_from_active_clients(my_info);
					my_info->is_connected = 0;
				}
			}
			free(msg);
		} else {
			printf("[OS] Connection closed");
			remove_from_active_clients(my_info);
		}
	}	
	pthread_exit(NULL);
}

int handle_cmd(char* cmd, char* rest, struct client_info_s* client) {
	int result = 0;
	printf("[ Object Store ] Parsing cmd %s, %s\n", cmd, rest);
	if (strcmp(cmd, "REGISTER") == 0) {
		if(!client->has_registered) {
			sscanf(rest, "%s \n", client->client_name); 
			if(strlen(client->client_name)== 0) { 
				return send_ko(client->client_fd, "REGISTER: No name provided");;
			}

			register_client(client);
			return send_ok(client->client_fd);
		} else {
			return send_ko(client->client_fd, "Client has already registered on the server!");
		}
	} else if(client->has_registered) {
		if (strcmp(cmd, "STORE") == 0) {
			char* last = NULL;
			size_t data_len = 0;
			char* name = strtok_r(rest, " ", &last);
			char* len_str = strtok_r(NULL, " ", &last);
			data_len = strtoul(len_str, NULL, 0);
			if (name == NULL || len_str == NULL || data_len == 0) {
				 return send_ko(client->client_fd, "STORE: Invalid arguments");
			}	
			char* data = (char*) malloc(sizeof(char) * data_len);
			if( read(client->client_fd, data, data_len) < 0) {
				free(data);
				return -1;
			}
			
			store_data(client, name, data_len, data);
			free(data);
			result = send_ok(client->client_fd);
		} else if(strcmp(cmd, "RETRIEVE") == 0) {
			char* name = strtok(rest, " \n");
			if (name == NULL) {
				return -1;
			}
			if (retrieve_data(client, name) != 0) {
				result = send_ko(client->client_fd, "Client doesn't have that object");
			}
		} else if(strcmp(cmd, "DELETE") == 0) {
			char* name = strtok(rest, " \n");
			if(name == NULL) {
				return -1;
			}
			if(delete_data(client, name) != 0) {
				result = send_ko(client->client_fd, "Client doesn't have that object");
			} else {
				result = send_ok(client->client_fd);
			}
		} else if(strcmp(cmd, "LEAVE") == 0) {
			result = disconnect_client(client);
		} else {
			result = send_ko(client->client_fd, "Unrecognised command!");
		}
	} else {
		result = send_ko(client->client_fd, "Client hasn't registered");
	}
	return result;
}

int register_client(struct client_info_s *client) {
	char path[PATH_MAX];
	sprintf(path, "%s/%s", "data", client->client_name);
	struct stat dir;
	if(stat(path, &dir) != 0) {
		mkdir(path, DEFAULT_MASK);
	}

	client->has_registered = 1;

}

int store_data(struct client_info_s *client, char* data_name, size_t data_len, char* data) {
	char path[PATH_MAX];
	sprintf(path, "%s/%s/%s", "data", client->client_name, data_name);
	
	FILE* file = fopen(path, "w");
	if(file == NULL) {
		perror("Error opening file");
		return -1;
	}

	size_t written = 0;
	while(written < data_len) {
		size_t now = fwrite(data, sizeof(char), (data_len - written), file);
		written += now;
	}
	fclose(file);
	return 0;
}

int delete_data(struct client_info_s *client, char* data_name) {
	char path[PATH_MAX];
	sprintf(path, "%s/%s/%s", "data", client->client_name, data_name);

	return unlink(path);
}

int retrieve_data(struct client_info_s *client, char* data_name) {
	char path[PATH_MAX];
	sprintf(path, "%s/%s/%s", "data", client->client_name, data_name);

	FILE* file = fopen(path, "r");
	if (file != NULL) {
		char buf[BUF_SIZE];
		char* read_data = NULL;
		size_t read_bytes = 0, read_now = 0;
		while((read_now = fread(buf, sizeof(char), BUF_SIZE, file)) > 0 ){
			read_data = (char*)realloc(read_data, sizeof(char) * (read_bytes + read_now) + 1 );
			memcpy(read_data + read_bytes, buf, read_now);
			read_bytes += read_now;
		}	
		read_data[read_bytes] = '\0';
		char msg[MAX_LINE_LENGTH];	
		sprintf(msg, "DATA %lu \n", read_bytes);

		if (writen(client->client_fd, msg, strlen(msg)) < 0) {
			return -1;
		}
		if (writen(client->client_fd, read_data, read_bytes) < 0) {
			return -1;
		}
		return 0;
	} else {
		perror("[ Object Store ] RETRIEVE could not open the file");
	}
	return -1;
}

void remove_from_active_clients(struct client_info_s* client) {
	pthread_mutex_lock(&server_info_mutex);
	if(client->prec != NULL) {
		client->prec->next = client->next;
	}
	if(client->next != NULL) {
		client->next->prec = client->prec;
	}

	if (server.clients_head == client) {
		server.clients_head = client->prec;
	}

	if(server.clients == client) {
		server.clients = client->next;
	}

	server.clients_connected --;
	pthread_mutex_unlock(&server_info_mutex);
}

int disconnect_client(struct client_info_s *client) {
	remove_from_active_clients(client);
	client->is_connected = 0;
	int result = send_ok(client->client_fd);
	close(client->client_fd);
	return result;
}

int send_ko(int fd, const char* msg) {
	char buf[BUF_SIZE];
	sprintf(buf, "KO %s \n", msg);
	return writen(fd, buf, strlen(buf));
}

int send_ok(int fd) {
	char msg[] = OK_STR;
	return writen(fd, OK_STR, strlen(msg));
}

