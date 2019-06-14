#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libobjstore.h"

#define EXITTHESHIP(msg) { perror(msg); exit(EXIT_FAILURE); }
#define ASSERT(expr) if(!(expr)) { EXITTHESHIP( "Assert failed: " #expr "\n");}
int main(int argc, char** argv) {
	char test[] = "Hey, vsauce! Michael here!";
	char name[] = "TestClient";	
	char store_name[] = "vsauce";
	ASSERT(os_connect(name) == 0); 

	ASSERT(os_store(store_name, test, strlen(test)) == OS_OK); 
	
	char* retrieve = (char*)os_retrieve(store_name);
	ASSERT(retrieve != NULL); 
	ASSERT(strcmp(retrieve, test) == 0);

	ASSERT(os_delete(store_name) == OS_OK);
	ASSERT(os_disconnect() == OS_OK);
}
