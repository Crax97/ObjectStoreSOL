#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commons.h"
#include "libobjstore.h"

#define ASSERT(expr) 													\
	if (!(expr)) { 														\
		fprintf(stderr, "[FAIL] %s failed test type %lu\n", name, test);\
		exit(EXIT_FAILURE); 											\
	} 																	\

#define ASSERT_IF(c, e) 	\
	ASSERT(c) 				\
	ASSERT(e) 				\

#define TEST_1_INITIAL_SIZE 100
#define TEST_1_FINAL_SIZE 100000
#define TEST_1_NUM_ROUNDS 20

#define LOREM_IPSUM "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nulla convallis suscipit quam. Aenean amet."
size_t num_assertions, num_passed, num_failed;

char*  names[TEST_1_NUM_ROUNDS];
char* datas[TEST_1_NUM_ROUNDS];
size_t lengths[TEST_1_NUM_ROUNDS];

void init_tests();
void end_tests(char* name, size_t test_type);
void str_repeat(char* buf, char* orig, size_t len, size_t newlen);

void print_usage(const char* name);

void test_1(char* name, size_t test);
void test_2(char* name, size_t test);
void test_3(char* name, size_t test);

int main(int argc, char** argv) {
	char *name = NULL;	
	size_t test = 0;

	if(argc < 2) {
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	};
	name = argv[1];
	test = strtoul(argv[2], NULL, 0); 

	init_tests();

	ASSERT(os_connect(name) == OS_OK); 

	switch(test) {
		case 1:
			test_1(name, test);
			break;
		case 2:
			test_2(name, test);
			break;
		case 3:
			test_3(name, test);
			break;
		default:
			print_usage(argv[0]);
			exit(EXIT_FAILURE);
			break;
	}
	ASSERT(os_disconnect() == OS_OK);
	end_tests(name, test);
	return EXIT_SUCCESS;
}

void print_usage(const char* name) {
	fprintf(stderr, "usage: %s <clientname> <test>\n", name); 
	fprintf(stderr, "\twhere test is an integer between 1 and 3\n");
}

void init_tests() {
	num_assertions = 0;
	num_failed = 0;
	num_passed = 0;

	const size_t increment = (TEST_1_FINAL_SIZE - TEST_1_INITIAL_SIZE) / TEST_1_NUM_ROUNDS;
	char ipsum[] = LOREM_IPSUM;

	size_t ips_len = strlen(ipsum);
	for(size_t i = 0; i < TEST_1_NUM_ROUNDS; i ++) {
		size_t newlen = ((i+1) * increment) + TEST_1_INITIAL_SIZE;
		char* data = (char*)calloc(newlen + 1, sizeof(char) );
		char name[30] = {0};
		sprintf(name, "Object%lu", i);		

		str_repeat(data, ipsum, ips_len, newlen);	
		names[i]= strdup(name);
		datas[i] = data;
		lengths[i] = newlen;
	}
}

void end_tests(char* name, size_t type) {
	for(size_t i = 0; i < TEST_1_NUM_ROUNDS; i ++) {
		free(names[i]);
		free(datas[i]);
	}

	printf("[OK] %s passed test type %lu\n", name, type);
}

void str_repeat(char* buf, char* orig,  size_t len, size_t newlen) {
	for(size_t i = 0; i < newlen; i ++) {
		buf[i] = orig[i % len];
	}	
	buf[newlen] = '\0';
}
void test_1(char* name, size_t test) {
	for(size_t i = 0; i < TEST_1_NUM_ROUNDS; i ++) {	
		ASSERT(os_store(names[i], datas[i], lengths[i]) == OS_OK);
	}
	
}

void test_2(char* name, size_t test) {
	for(size_t i = 0; i < TEST_1_NUM_ROUNDS; i ++) {
		char* retrieved = (char*)os_retrieve(names[i]);
		ASSERT_IF(retrieved != NULL, strlen(retrieved) == lengths[i]);
		ASSERT_IF(retrieved != NULL, strcmp(retrieved, datas[i]) == 0);
		free(retrieved);
	}
}

void test_3(char* name, size_t test) {
	for(size_t i = 0; i < TEST_1_NUM_ROUNDS; i ++) {
		ASSERT(os_delete(names[i]) == OS_OK);
	}
}

