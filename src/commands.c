#include "commands.h"
#include "worker.h"
#include "commons.h"
#include "server.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>

int handle_register(char* name, struct worker_s* worker, struct server_info_s*  info) {
	if(worker == NULL) {
		CATASTROPHIC_FAILURE("Worker is NULL in REGISTER!!!!");
	}
	if (name == NULL) {
		return ko("REGISTER: Invalid arguments", worker);
	}

	if(worker->is_registered) {
		return ko("REGISTER: Client is already registered", worker);	
	}

	if(!is_worker_name_unique(name)) {
		return ko("REGISTER: Requested name is not unique", worker);;
	}
	accept_worker(name, worker, info);	
	return ok(worker);
}

int handle_store(char* data_name, char* data_len, struct worker_s* worker) {
	if(worker == NULL) {
		CATASTROPHIC_FAILURE("Worker is NULL in STORE");
	}
	if(data_name == NULL || data_len == NULL) {
		return ko("STORE: Invalid arguments", worker);
	}
	ssize_t len = strtoul(data_len, NULL, 0);

	char path[PATH_MAX];
	sprintf(path, "%s/%s/%s", DATA_FOLDER, worker->associated_name, data_name);
	
	char *buf = (char*)calloc(len, sizeof(char));
	if(read_data(worker->worker_fd, buf, len) < len) {
		return ko("Error reading data from socket!", worker);
	}

	if(write_to_disk(path, buf, len) < len) {
		free(buf);
		return ko("Error writing data!", worker);
	}

	free(buf);
	return ok(worker);
}

int handle_retrieve(char* data_name, struct worker_s *worker) {
	if(worker == NULL) {
		CATASTROPHIC_FAILURE("Worker is null in RETRIEVE!");
	}

	if(data_name == NULL) {
		return ko("RETRIEVE: Invalid arguments", worker);
	}

	char* data = NULL;	
	char path[PATH_MAX];
	sprintf(path, "%s/%s/%s", DATA_FOLDER, worker->associated_name, data_name);
	ssize_t bytes_read = 0;
	if((bytes_read =read_from_disk(path, &data)) <= 0) {
		return ko("RETRIEVE: Could not read file. Are you sure you own it?", worker);
	}	

	char header[30];
	sprintf(header, DATA_STR, (unsigned long long)bytes_read);
	ssize_t header_len = strlen(header);
	
	if(writen(worker->worker_fd, header, header_len) < header_len) {
		free(data);
		return OS_FALSE;
	}

	if(writen(worker->worker_fd, data, bytes_read) < bytes_read) {
		free(data);
		return OS_FALSE;
	}

	free(data);
	return OS_TRUE;
}

int handle_delete(char* data_name, struct worker_s* worker) {
	if(worker == NULL) {
		CATASTROPHIC_FAILURE("Worker is NULL in DELETE!");
	}
	
	if(data_name == NULL) {
		return ko("DELETE: Invalid arguments", worker);
	}

	char path[PATH_MAX];
	sprintf(path, "%s/%s/%s", DATA_FOLDER, worker->associated_name, data_name);
	int deleted = delete_file(path);
	if(!deleted) {
		return ko("DELETE: Could not delete the file! Do you own it?", worker);
	}
	return ok(worker);
}

int handle_leave(struct worker_s* worker, struct server_info_s* server) {
	if (worker == NULL) {
	CATASTROPHIC_FAILURE("Worker is NULL in LEAVE!");
}

	stop_worker(worker, server);
	return OS_TRUE;
}

int ok(struct worker_s* worker) {
	int ok_len = strlen(OK_STR);
	if (writen(worker->worker_fd, OK_STR, ok_len) < ok_len) {
		return OS_FALSE;
	}
	return OS_TRUE;
}

int ko(char* msg, struct worker_s *worker) {
	char buf[PATH_MAX + 1];
	sprintf(buf, "KO %s \n", msg);
	fprintf(stderr, "Sending KO to %s-%d, msg: %s\n", worker->associated_name, worker->worker_fd, msg);
	
	ssize_t len = strlen(buf);
	if(writen(worker->worker_fd, buf, len) < len) {
		return OS_FALSE;
	}
	return OS_TRUE;

}
