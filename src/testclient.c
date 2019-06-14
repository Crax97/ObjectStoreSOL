#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libobjstore.h"

#define EXITTHESHIP(msg) { perror(msg); exit(EXIT_FAILURE); }
int main(int argc, char** argv) {
	char test[] = "Hey, vsauce! Michael here!";
	char name[] = "TestClient";	
	char store_name[] = "vsauce";
	if(os_connect(name) != 0) {
		EXITTHESHIP("Connection failed");
	}

	if(os_store(store_name, test, strlen(test)) != OS_OK) {
		EXITTHESHIP("Store failed");
	}

	char* retrieve = (char*)os_retrieve(store_name);
	if(retrieve == NULL) {
		EXITTHESHIP("Retrieve failed");
	}

	if(os_delete(store_name) != OS_OK) {
		EXITTHESHIP("Delete failed");
	}
	if(os_disconnect() != OS_OK) {
		EXITTHESHIP("Disconnect failed");
	}
}
