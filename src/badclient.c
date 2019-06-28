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
#include <signal.h>

#define ERR_MSG_FORMAT "[Object Store] Err: %s\n"

int os_connect(char* name);
int os_store(char* name, void* block, size_t len);
void* os_retrieve(char* name);
int os_delete(char* name);
int os_disconnect();

int client_sock = 0;

int main(int argc, char** argv) {
	//Simulating death while registering
	struct sockaddr_un addr;
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, SOCKNAME);

	if ((client_sock = socket(AF_UNIX, SOCK_STREAM, 0)) <= 0) {
		return OS_ERR;
	}
	if (connect(client_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		return OS_ERR;
	}	

	char buf[MAX_LINE_LENGTH + 1];
	sprintf(buf, REGISTER_STR, "HatKid");
	ssize_t msglen = strlen(buf);

	writen(client_sock, buf, 5);
	close(client_sock); // DOWN WITH THE MAFIAAA c:<	
	client_sock = 0;
	printf("Test 1 OK\n");

	//Simulating death while writing stuff
	memset(buf, 0, 30);
	SC(os_connect("MustacheKid"));
	sprintf(buf, STORE_STR, "wontsave", 15L);
	SC(writen(client_sock, buf, strlen(buf)));
	close(client_sock); // BABEM c:<
	client_sock = 0;
	printf("Test 2 OK\n");

	return 0;
}

const char* get_ko_msg(char* buf);

int os_connect(char* name) {
	if(client_sock == 0) {
		struct sockaddr_un addr;
		addr.sun_family = AF_UNIX;
		strcpy(addr.sun_path, SOCKNAME);

		if ((client_sock = socket(AF_UNIX, SOCK_STREAM, 0)) <= 0) {
			return OS_ERR;
		}
		if (connect(client_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
			return OS_ERR;
		}	

		char buf[MAX_LINE_LENGTH + 1];
		sprintf(buf, REGISTER_STR, name);
		ssize_t msglen = strlen(buf);

		if(writen(client_sock, buf, msglen) < msglen) {
			return OS_ERR;
		}

		char* response = NULL;
	   	if(read_to_newline(client_sock, &response) < 0) {
			free(response);
			return OS_ERR;
		}

		if(strcmp(response, OK_STR) == 0) {
			return OS_OK;	
		} else {
			const char* ko_msg = get_ko_msg(response);
			fprintf(stderr, ERR_MSG_FORMAT, ko_msg);
		}
		free(response);
	}
	return OS_ERR;
}

int os_store(char* name, void* block, size_t len) {
    if(client_sock < 0) {
		return OS_NOCONN;
	}
	if (block != NULL) {
	
		char buf[MAX_LINE_LENGTH + 1];
		sprintf(buf, STORE_STR, name, len);
		ssize_t msglen = strlen(buf);

		if (writen(client_sock, buf, msglen) < msglen) {
			return OS_ERR;
		};
		if (writen(client_sock, (char*)block, len) < (ssize_t) len) {
			return OS_ERR;
		}

		char* msg = NULL;
		if( read_to_newline(client_sock, &msg) < 0) {
			free(msg);
			return OS_ERR;
		}

		if(strcmp(msg, OK_STR) != 0) {
			const char* ko_msg = get_ko_msg (msg);
			fprintf(stderr, ERR_MSG_FORMAT, ko_msg); 
			free(msg);
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
		ssize_t msglen = strlen(buf);
		if(writen(client_sock, buf, msglen) < msglen) {
			return OS_ERR;
		}
		
		char* msg = NULL;
	   	if(read_to_newline(client_sock, &msg) < 0) {
			return OS_ERR;
		}

		long long datalen = 0;
		if (sscanf(msg, DATA_STR, &datalen) > 0) {
			data = (char*) calloc(datalen + 1, sizeof(char));
			if (read_data(client_sock, data, datalen) < datalen) {
				free(data);
				return OS_ERR;
			}
		} else {
			const char* ko_msg = get_ko_msg(msg);
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
		ssize_t msglen = strlen(buf);
		if (writen(client_sock, buf, msglen) < msglen) {
			return OS_ERR;
		}
	
		char* msg = NULL;
		if(read_to_newline(client_sock, &msg) < 0) {
			free(msg);
			return OS_ERR;
		}

		if(strcmp(msg, OK_STR) == 0) {
			free(msg);
			return OS_OK;
		}	
		const char* err_msg = get_ko_msg(msg);
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
	ssize_t msglen = strlen(LEAVE_MSG);
	writen(client_sock, LEAVE_MSG, msglen); // As from the specification, os_disconnect() must always be successful. Ignorning eventual erros
	char* msg = NULL;
	read_to_newline(client_sock, &msg); 
	if(msg != NULL) free(msg);
	close(client_sock);
	client_sock = 0;
	return OS_OK;

}

const char* get_ko_msg(char* msg) {
	char* ko_tok = strtok(msg, " ");
	if(strcmp(ko_tok, "KO") == 0) {
		return (msg + 3);
	}
	fprintf(stderr, ERR_MSG_FORMAT, "Buffer didn't contain a KO message");
	exit(EXIT_FAILURE);
	return NULL;
}
