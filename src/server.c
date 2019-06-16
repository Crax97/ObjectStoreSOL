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
#include <sys/dir.h>
#include <limits.h>
#include <errno.h>
#include <fcntl.h>

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
	int fd;
	struct client_info_s* clients;
	struct client_info_s* clients_head;
	size_t clients_connected;
};

pthread_mutex_t server_info_mutex = PTHREAD_MUTEX_INITIALIZER;
struct server_info_s server;
int SERVER_RUNNING = 1;

void remove_from_active_clients(struct client_info_s* client);
void sigusr_handler_f(int sig);
void sigint_handler_f(int sig);
void sigpipe_handler_f(int sig);
void handle_exit(void);
void close_everyone(void);

void* thread_worker(void* args);
void* signal_thread_worker(void* args);
int send_ok(int fd);
int send_ko(int fd, const char* msg);
int handle_cmd(char* msg, struct client_info_s *client);
int check_client_name_unique(char* name);

// Server stuff
int register_client(struct client_info_s *client);
int store_data(struct client_info_s *client, char* data_name, size_t data_len, char* data);
int retrieve_data(struct client_info_s *client, char* data_name);
int delete_data(struct client_info_s *client, char* data_name);
int disconnect_client(struct client_info_s *client);

int main(int argc, char** argv) {
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

	sigset_t signals;
	sigemptyset(&signals);
	pthread_sigmask(SIG_SETMASK, &signals, NULL);

	pthread_t signal_thread;
	SC(pthread_create(&signal_thread, NULL, signal_thread_worker, NULL));


	while(1) {
		int client_fd = 0;
		socklen_t len;
		SC(client_fd = accept(socket_fd, (struct sockaddr*)&sock, &len));
		
		create_worker_for_fd( &server, client_fd );
	}

}

void handle_exit() {
	SERVER_RUNNING = 0;
	shutdown(server.fd, SHUT_RDWR);
	printf("[Object Store] Waiting for all the workers to end their job\n");
	close_everyone();

	while(server.clients_connected > 0) {
		//Waiting for all the threads to finish their work	
	}
	printf("[Object Store] Bye bye!\n");
	unlink(SOCKNAME);
	exit(EXIT_SUCCESS);
}

void create_worker_for_fd(struct server_info_s *info, int client_fd) {
	struct client_info_s* client = (struct client_info_s*)  malloc(sizeof(struct client_info_s));
	pthread_mutex_lock(&server_info_mutex);

	client->has_registered = 0;
	client->is_connected = 1;
	client->client_fd = client_fd;
	client->next = NULL;
	client->prec = info->clients_head;

	if(server.clients == NULL) {
		server.clients = client;
	}
	server.clients_connected ++;

	pthread_mutex_unlock(&server_info_mutex);

	pthread_t worker_thread;
	SC(pthread_create(&worker_thread, NULL, thread_worker, client)); 
	SC(pthread_detach(worker_thread));
	client->client_thread = worker_thread;
	printf("[Object Store] Got a new Client, assigning fd  %d\n", client_fd);
}

void* signal_thread_worker(void* args) {
	sigset_t set;
	sigfillset(&set);
	pthread_sigmask(SIG_SETMASK, &set, NULL);	

	// This warning is a bug
	struct sigaction sigusr1_handler = {0};
	sigusr1_handler.sa_handler = sigusr_handler_f;
	sigusr1_handler.sa_flags = SA_RESTART;
	sigaction(SIGUSR1, &sigusr1_handler, NULL);

	struct sigaction sigint_handler = {0};
	sigint_handler.sa_handler = sigint_handler_f;
	sigint_handler.sa_flags = SA_RESTART;
	sigaction(SIGINT, &sigint_handler, NULL);

	struct sigaction sigpipe_handler = {0};
	sigpipe_handler.sa_handler = sigpipe_handler_f;
	sigpipe_handler.sa_flags = SA_RESTART;
	sigaction(SIGPIPE, &sigpipe_handler, NULL);

	while(SERVER_RUNNING) {
		int signal;
		sigwait(&set, &signal);
	   	printf("[ Object Store ] Received signal %d\n", signal);	
	}

	pthread_exit(NULL);
}
void sigusr_handler_f(int sig) {
	server_print_info(&server);
}

void sigpipe_handler_f(int sig) {
	printf("[Object Store] received SIGPIPE, someone died\n");
}

void sigint_handler_f(int sig) {
	printf("[Object Store] Received SIGINT, starting quit procedure\n");
	handle_exit();
}

void gather_directory_elts_and_size(DIR* dir, size_t* elts, size_t* size) {
	char buf[PATH_MAX];
	getcwd(buf, PATH_MAX);
	printf("scanning dir %s\n", buf); 
	struct dirent* cur_dir = readdir(dir);
	while(cur_dir != NULL) {
		if( strcmp(cur_dir->d_name, ".") == 0 ||
			strcmp(cur_dir->d_name, "..") == 0) {
		} else {
			if(cur_dir->d_type == DT_DIR) {
				DIR* newdir = opendir(cur_dir->d_name);
				SC(chdir(cur_dir->d_name));	
				if(newdir != NULL) {
					gather_directory_elts_and_size(newdir, elts, size);	
					closedir(newdir);
				}
			} else {
				struct stat info;
				if(stat(cur_dir->d_name, &info) == 0) {

					(*elts) += 1;
					(*size) += info.st_size;	
				} else {
					perror("Stat failure");
				}
			}
		}
		cur_dir = readdir(dir);
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
		printf("[Object Store] Clients connected: %lu\n", info->clients_connected);
		printf("[Object Store] Total size: %lu\n", total_dim);
		printf("[Object Store] Total elements: %lu\n", total_count);	
	} else {
		fprintf(stderr, "[Object Store] could not open the data directory!");
		exit(EXIT_FAILURE);
	}
}

void* thread_worker(void* args) {
	struct client_info_s* my_info = (struct client_info_s*)args;
	while(SERVER_RUNNING && my_info->is_connected) {
		char* msg = read_to_newline(my_info->client_fd);
		errno = 0;
		
		if(msg != NULL && strlen(msg) > 0) {
			int result = handle_cmd(msg, my_info);
			if(result < 0) {
				printf("[Object Store] Something failed while trying to communicate with %d. Closing connection\n", my_info->client_fd);
				close(my_info->client_fd);
				my_info->is_connected = 0;
			}
		} else {
			printf("[Object Store] Socket for %s was closed\n", my_info->client_name);
			remove_from_active_clients(my_info);
			close(my_info->client_fd);
			my_info->is_connected = 0;
		}

	}	
	printf("[Object Store] Thread worker for %s %d ended\n", my_info->client_name, my_info->client_fd);
	pthread_exit(NULL);
}

int check_client_name_unique(char* name) {
	struct client_info_s* current_client = server.clients;
	while(current_client != NULL) {
		if(strcmp(name, current_client->client_name) == 0) {
			return OS_ERR;
		}
		current_client = current_client->next;
	}
	return OS_OK;
}

int handle_cmd(char* msg, struct client_info_s* client) {

	char* last = NULL;
	char* cmd = strtok_r(msg, " ", &last);
	if(cmd != NULL) {
		printf("[Object Store] Handling %s from %s[%d]\n", cmd, client->has_registered ? client->client_name : "[Not registered yet]", client->client_fd);
		if (strcmp(cmd, "REGISTER") == 0) {
			if(client->has_registered == 0) {
				char* name = strtok_r(NULL, " \n", &last);
				if(name == NULL || strlen(name) == 0) {
					return send_ko(client->client_fd, "REGISTER: No name provided");;
				}

				if(check_client_name_unique(name) < 0) {
					printf("[Object Store] Client %d is already registered on the server with the name %s\n", client->client_fd, name);
					return send_ko(client->client_fd, "REGISTER: Client is already registered on the server");
				}
				strcpy(client->client_name, name);
				SC(register_client(client));
				return send_ok(client->client_fd);
			} else {
				return send_ko(client->client_fd, "Client has already registered on the server!");
			}
		} else if(client->has_registered) {
			if (strcmp(cmd, "STORE") == 0) {
				char* data_name = strtok_r(NULL, " ", &last);
				char* len_str = strtok_r(NULL, " ", &last);
				size_t data_len = data_len = strtoul(len_str, NULL, 0);	
				
				if (data_name == NULL || len_str == NULL || data_len == 0) {
					 return send_ko(client->client_fd, "STORE: Invalid arguments");
				}	

				char* data = read_data(client->client_fd, data_len);
				if(data == NULL) {
					return send_ko(client->client_fd, "STORE: No data sent");
				}
				if(!store_data(client, data_name, data_len, data)) {
					fprintf(stderr, "[Object Store] Failed storing data for %s\n", client->client_name);
				}
				free(data);
				return send_ok(client->client_fd);
			} else if(strcmp(cmd, "RETRIEVE") == 0) {
				char* data_name = strtok_r(NULL, " \n", &last);
				if (data_name == NULL) {
					return send_ko(client->client_fd, "Client didn't send the name of the object");
				}
				if (!retrieve_data(client, data_name)) {
					return send_ko(client->client_fd, "Client doesn't have that object");
				}
			} else if(strcmp(cmd, "DELETE") == 0) {
				char* data_name = strtok_r(NULL, " \n", &last);
				if(data_name == NULL) {
					return send_ko(client->client_fd, "Client didn't send the name of the object to delete");
				}
				if(!delete_data(client, data_name)) {
					return send_ko(client->client_fd, "Client doesn't have that object");
				} else {
					return send_ok(client->client_fd);
				}
			} else if(strcmp(cmd, "LEAVE") == 0) {
				return disconnect_client(client);
				free(client);
			} else {
				return send_ko(client->client_fd, "Unrecognised command!");
			}
		} else {
			return send_ko(client->client_fd, "Client hasn't registered");
		}
	}
	return 0;
}

// Make it so that it retunrns != 1 only if ther's an unrecoverable error
int register_client(struct client_info_s *client) {
	char path[PATH_MAX];
	sprintf(path, "%s/%s", "data", client->client_name);
	struct stat dir;
	if(stat(path, &dir) != 0) {
		mkdir(path, DEFAULT_MASK);
	}
	client->has_registered = 1;
	return OS_OK;
}

int store_data(struct client_info_s *client, char* data_name, size_t data_len, char* data) {
	char path[PATH_MAX];
	sprintf(path, "%s/%s/%s", "data", client->client_name, data_name);
	
	FILE* file = fopen(path, "w");
	if(file == NULL) {
		perror("Error opening file");
		return OS_ERR;
	}

	size_t written = 0;
	while(written < data_len) {
		size_t now = fwrite(data, sizeof(char), data_len, file);
		written += now;
	}
	fclose(file);
	return OS_OK;
}

int delete_data(struct client_info_s *client, char* data_name) {
	char path[PATH_MAX];
	sprintf(path, "%s/%s/%s", "data", client->client_name, data_name);

	struct stat info;
	if(stat(path, &info) == 0) {
		return unlink(path) == 0;
	} else {
		return OS_ERR;
	}
}

int retrieve_data(struct client_info_s *client, char* data_name) {
	char path[PATH_MAX];
	sprintf(path, "%s/%s/%s", "data", client->client_name, data_name);

	FILE* file = fopen(path, "r");
	if (file != NULL) {
		struct stat info;
		SC(stat(path, &info));
		
		size_t len = info.st_size;
		char* data = (char*) calloc(len, sizeof(char));	

		while(len > 0) {
			size_t now = fread(data, sizeof(char), len, file);
			len -= now;
		}
		char data_header[30];
		sprintf(data_header, DATA_STR, (unsigned long)info.st_size);
		if(writen(client->client_fd, data_header, strlen(data_header)) < 0) {
			return OS_ERR;
		}	
	
		if(writen(client->client_fd, data, info.st_size) < 0) {
			return OS_ERR;
		}
		free(data);
		return OS_OK;
	} else {
		fprintf(stderr, "[Object Store] RETRIEVE could not open the file");
	}
	return OS_ERR;
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
};

void close_everyone() {
	struct client_info_s* client = server.clients;
	while(client != NULL) {
		SC(close(client->client_fd));
		client = client->next;
	}
}

int disconnect_client(struct client_info_s *client) {
	client->is_connected = 0;
	int result = send_ok(client->client_fd);
	close(client->client_fd);
	remove_from_active_clients(client);
	return result;
}

int send_ko(int fd, const char* msg) {
	fprintf(stderr, "[Object Store] KO to %d: %s\n", fd, msg);
	char buf[BUF_SIZE];
	sprintf(buf, "KO %s \n", msg);
	return writen(fd, buf, strlen(buf));
}

int send_ok(int fd) {
	char msg[] = OK_STR;
	return writen(fd, OK_STR, strlen(msg));
}

