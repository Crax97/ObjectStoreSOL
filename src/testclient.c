#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commons.h"
#include "libobjstore.h"

#define EXITTHESHIP(msg) { fprintf(stderr, msg); exit(EXIT_FAILURE); }
#define ASSERT(expr) if(!(expr)) { EXITTHESHIP( "Assert failed: " #expr );}

#define TEST_1_INITIAL_SIZE 100
#define TEST_1_FINAL_SIZE 100000
#define TEST_1_NUM_ROUNDS 20

#define LOREM_IPSUM "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nulla convallis suscipit quam. Aenean amet."
char*  names[TEST_1_NUM_ROUNDS];
char* datas[TEST_1_NUM_ROUNDS];
size_t lengths[TEST_1_NUM_ROUNDS];

void init_tests();
void end_tests();
void str_repeat(char* buf, char* orig, size_t len, size_t newlen);

void test_1();
void test_2();
void test_3();

int main(int argc, char** argv) {
	char *name = NULL;	
	size_t test = 0;

	if(argc < 2) {
		fprintf(stderr, "Usage: %s name test\n", argv[0]);
		exit(EXIT_FAILURE);
	};
	name = argv[1];
	test = strtoul(argv[2], NULL, 0); 

	init_tests();

	ASSERT(os_connect(name) == OS_OK); 

	switch(test) {
		case 1:
			test_1();
			break;
		case 2:
			test_2();
			break;
		case 3:
			test_3();
			break;
		default:
			fprintf(stderr, "Test must be 1, 2, or 3\n");
			break;
	}
	ASSERT(os_disconnect() == OS_OK);
	end_tests();
	return EXIT_SUCCESS;
}

void init_tests() {
	const size_t increment = (TEST_1_FINAL_SIZE - TEST_1_INITIAL_SIZE) / TEST_1_NUM_ROUNDS;
	char ipsum[] = LOREM_IPSUM;

	size_t ips_len = strlen(ipsum);
	for(size_t i = 0; i < TEST_1_NUM_ROUNDS; i ++) {
		size_t newlen = (i * increment) + TEST_1_INITIAL_SIZE;
		char* data = (char*)calloc(newlen + 1, sizeof(char) );
		char name[30] = {0};
		sprintf(name, "Object%lu", i);		

		str_repeat(data, ipsum, ips_len, newlen);	
		names[i]= strdup(name);
		datas[i] = data;
		lengths[i] = newlen;
	}
}

void end_tests() {
	for(size_t i = 0; i < TEST_1_NUM_ROUNDS; i ++) {
		free(names[i]);
		free(datas[i]);
	}
}

void str_repeat(char* buf, char* orig,  size_t len, size_t newlen) {
	for(size_t i = 0; i < newlen; i ++) {
		buf[i] = orig[i % len];
	}	
	buf[newlen] = '\0';
}

void test_1() {
	for(size_t i = 0; i < TEST_1_NUM_ROUNDS; i ++) {	
		ASSERT(os_store(names[i], datas[i], lengths[i]) == OS_OK);
	}
	
}

void test_2() {
	for(size_t i = 0; i < TEST_1_NUM_ROUNDS; i ++) {
		char* retrieved = (char*)os_retrieve(names[i]);
		ASSERT(strlen(retrieved) == lengths[i]);
		ASSERT(strcmp(retrieved, datas[i]) == 0);
		free(retrieved);
	}
}

void test_3() {
	for(size_t i = 0; i < TEST_1_NUM_ROUNDS; i ++) {
		ASSERT(os_delete(names[i]) == OS_OK);
	}
}

