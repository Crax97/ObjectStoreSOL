#include "server.h"
#include "commons.h"
#include "worker.h"
#include "signal.h"

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

pthread_cond_t server_disconnect = PTHREAD_COND_INITIALIZER;

int main(int argc, char** argv) {
	unlink(SOCKNAME); // Just in case

	struct sockaddr_un sock;	
	sock.sun_family = AF_UNIX;
	strcpy(sock.sun_path, SOCKNAME);			

	int socket_fd = 0;
	SC(socket_fd = socket(AF_UNIX, SOCK_STREAM, 0));
	SC(bind(socket_fd, (struct sockaddr*)&sock, sizeof(sock)));
	SC(listen(socket_fd, BACKLOG));

	create_folder(DATA_FOLDER);

	struct server_info_s server;
	server.active_clients = 0;
	server.server_running = OS_TRUE;
	server.server_fd = socket_fd;
    
	pthread_t signal_thread = create_signal_thread(&server);

	printf(OS "Ready\n");

	struct pollfd fd;
	fd.fd = socket_fd;
	fd.events = POLLIN;

	while(server.server_running) {

		if(poll(&fd, 1, 10) >= 1) {

			int client_fd = 0;
			socklen_t len;
			client_fd = accept(socket_fd, (struct sockaddr*)&sock, &len);
			
			if(client_fd > 0) {
				printf(OS "Got a new client on fd %d\n", client_fd);
				struct worker_s* new_worker = create_worker(client_fd, &server);
				add_worker_list(new_worker);
			}
		}
	}

	SC(pthread_join(signal_thread, NULL));

	while(server.active_clients > 0 ) {
		sched_yield();	
	}

	printf(OS "Bye bye!\n");

	unlink(SOCKNAME);
	return EXIT_SUCCESS;
}
