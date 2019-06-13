#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "commons.h"

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

	while(1) {
		int client_fd = 0;
		socklen_t len;
		SC(client_fd = accept(socket_fd, (struct sockaddr*)&sock, &len));

		printf("Got a new client! %d\n", client_fd);

	}

}

void* thread_worker(void* args) {
	int fd = *(int*)args;
	struct client_s client;
	char buf[MAX_LINE_LENGTH + 1];
	char* cmd = NULL;
	while(1) {
		read(fd, buf, MAX_LINE_LENGTH);
		cmd = strtok(buf, " ");
		if(cmd != NULL) {
			
		} else {
			printf("CMD received: %s\n", cmd);
            
		}
	}

	pthread_exit(NULL);
}
