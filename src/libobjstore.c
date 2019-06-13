#include "libobjstore.h"
#include "commons.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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

	char buf[MAX_LINE_LENGTH + 1];
	sprintf(buf, REGISTER_STR, name);
	size_t msglen = strlen(buf);
	SC(write(client_sock, &msglen, sizeof(size_t)));
	SC(writen(client_sock, buf, msglen));

	SC(read(client_sock, &msglen, sizeof(size_t)));
	SC(readn(client_sock, buf, msglen));

	if(strcmp(buf, OK_STR) == 0) {
		return OS_OK;	
	} else {
		const char* ko_msg = get_ko_msg(buf);
		fprintf(stderr, ERR_MSG_FORMAT, ko_msg);
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
		size_t msglen = strlen(buf);

		SC(write(client_sock, &msglen, sizeof(size_t)));	
		SC(writen(client_sock, buf, msglen));

		SC(read(client_sock, &msglen, sizeof(size_t)));
		SC(readn(client_sock, buf, msglen));

		if(strcmp(buf, OK_STR) != 0) {
			const char* ko_msg = get_ko_msg (buf);
			fprintf(stderr, ERR_MSG_FORMAT, ko_msg); 
			return OS_ERR;
		}
	}
	return OS_OK;
}

void* os_retrieve(char* name) {
	if (client_sock <= 0) {
		return NULL;
	}
	if (name != NULL) {
		char buf[MAX_LINE_LENGTH];
		sprintf(buf, RETRIEVE_STR, name);
		size_t msglen = strlen(buf);
		SC(write(client_sock, &msglen, sizeof(size_t)));
		SC(writen(client_sock, buf, msglen));
		
		//Message is split in two parts: result\n [data | KO msg \n]
		SC(read(client_sock, &msglen, sizeof(size_t)));
		SC(readn(client_sock, buf, msglen));

		size_t datalen = 0;
		if (sscanf(buf, DATA_STR, &datalen) > 0) {
			char* data = (char*) malloc(sizeof(char) * datalen);	
			SC(readn(client_sock, data, datalen));
			return data;
		} else {
			const char* ko_msg = get_ko_msg(buf);
			fprintf(stderr, ERR_MSG_FORMAT, ko_msg);
		}
	}
	return NULL;
}

int os_delete(char* name) {
	if(client_sock <= 0) {
		return OS_NOCONN;
	}

	if(name != NULL) {
		char buf[MAX_LINE_LENGTH + 1];
		sprintf(buf, DELETE_STR, name);
		size_t msglen = strlen(buf);
		SC(write(client_sock, &msglen, sizeof(size_t)));
		SC(writen(client_sock, buf, msglen));
		
		SC(read(client_sock, &msglen, sizeof(size_t)));
		SC(readn(client_sock, buf, msglen));
		if(strcmp(buf, OK_STR) == 0) {
			return OS_OK;
		}	
		const char* err_msg = get_ko_msg(buf);
		fprintf(stderr, ERR_MSG_FORMAT, err_msg);
	}

	return OS_ERR;
}

int os_disconnect() {
	if(client_sock <= 0) {
		return OS_NOCONN;
	}

	const char* LEAVE_MSG = LEAVE_STR;
	char buf[MAX_LINE_LENGTH + 1];
	size_t msglen = strlen(LEAVE_MSG);
	SC(write(client_sock, &msglen, sizeof(size_t)));
	SC(writen(client_sock, LEAVE_MSG, msglen));
   
	SC(read(client_sock, &msglen, sizeof(size_t)));	
	SC(readn(client_sock, buf, msglen)); 

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
