#include "libobjstore.h"
#include "commons.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>

#define ERR_MSG_FORMAT "[Object Store] Err: %s\n"

int client_sock = 0;

const char* get_ko_msg(char* buf);

int os_connect(char* name) {
	struct sockaddr_un addr;
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, SOCKNAME);

	if ((client_sock = socket(AF_UNIX, SOCK_STREAM, 0)) <= 0) {
		return OS_ERR;
	}
	if (connect(client_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		return OS_ERR;
	}	

	if(fcntl(client_sock, F_SETFL, O_NONBLOCK) == -1) {
		perror("Failed to set nonblocking IO on client socked");
		return OS_ERR;
	}

	char buf[MAX_LINE_LENGTH + 1];
	sprintf(buf, REGISTER_STR, name);
	size_t msglen = strlen(buf);

	SC(writen(client_sock, buf, msglen));

	char* msg = read_to_newline(client_sock);

	if(strcmp(msg, OK_STR) == 0) {
		return OS_OK;	
	} else {
		const char* ko_msg = get_ko_msg(msg);
		fprintf(stderr, ERR_MSG_FORMAT, ko_msg);
	}
	free(msg);
	return OS_ERR;
}

int os_store(char* name, void* block, size_t len) {
    if(client_sock < 0) {
		return OS_NOCONN;
	}
	if (block != NULL) {
	
		char buf[MAX_LINE_LENGTH + 1];
		sprintf(buf, STORE_STR, name, len);
		size_t msglen = strlen(buf);

		SC(writen(client_sock, buf, msglen));
		SC(writen(client_sock, (char*)block, len));

		char* msg = read_to_newline(client_sock);

		if(strcmp(msg, OK_STR) != 0) {
			const char* ko_msg = get_ko_msg (msg);
			fprintf(stderr, ERR_MSG_FORMAT, ko_msg); 
			return OS_ERR;
		}
		free(msg);
	}
	return OS_OK;
}

void* os_retrieve(char* name) {
	char* data = NULL;
	if (client_sock <= 0) {
		return NULL;
	}
	if (name != NULL) {
		char buf[MAX_LINE_LENGTH];
		sprintf(buf, RETRIEVE_STR, name);
		size_t msglen = strlen(buf);
		SC(writen(client_sock, buf, msglen));
		
		char* msg = read_to_newline(client_sock);

		size_t datalen = 0;
		if (sscanf(msg, DATA_STR, &datalen) > 0) {
			//TODO change to true readn
			data = read_data(client_sock, datalen);
		} else {
			const char* ko_msg = get_ko_msg(buf);
			fprintf(stderr, ERR_MSG_FORMAT, ko_msg);
		}
		free(msg);
	}
	return (void*)data;
}

int os_delete(char* name) {
	if(client_sock <= 0) {
		return OS_NOCONN;
	}

	if(name != NULL) {
		char buf[MAX_LINE_LENGTH + 1];
		sprintf(buf, DELETE_STR, name);
		size_t msglen = strlen(buf);
		SC(writen(client_sock, buf, msglen));
	
		char* msg = read_to_newline(client_sock);
		if(strcmp(msg, OK_STR) == 0) {
			free(msg);
			return OS_OK;
		}	
		const char* err_msg = get_ko_msg(buf);
		fprintf(stderr, ERR_MSG_FORMAT, err_msg);
		free(msg);
	}

	return OS_ERR;
}

int os_disconnect() {
	if(client_sock <= 0) {
		return OS_NOCONN;
	}
	const char* LEAVE_MSG = LEAVE_STR;
	size_t msglen = strlen(LEAVE_MSG);
	SC(writen(client_sock, LEAVE_MSG, msglen));
	char* msg = read_to_newline(client_sock); 
	free(msg);
	return OS_OK;

}

const char* get_ko_msg(char* buf) {
	char* ko_tok = strtok(buf, " ");
	if(strcmp(ko_tok, "KO") == 0) {
		return (buf + 3);
	}
	fprintf(stderr, ERR_MSG_FORMAT, "Buffer didn't contain a KO message");
	exit(EXIT_FAILURE);
	return NULL;
}
