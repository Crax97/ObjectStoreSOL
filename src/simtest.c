#include "libobjstore.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SC(e) if ((e) < 0) {fprintf(stderr, "Test failed " #e "\n"); exit(EXIT_FAILURE); }
#define A(e) if( !(e)) {fprintf(stderr, "Assert failed " #e "\n"); exit(EXIT_FAILURE);}

#define STR "PaninoAlTonno"

typedef struct complex_s {
	int num;
	char str[20];
	long lnum;
} complex;

int main(int argc, char** argv) {
	SC(os_connect("RealWorldSimulation"));
	int n = 10;
	SC(os_store("number", (void*)&n, sizeof(int)));
	int* result = (int*) os_retrieve("number");
	A(result != NULL);
	A(*result == n);

	int date[] = {9, 4, 1997};
	SC(os_store("date", date, sizeof(int) * 3));
	int* date_r = (int*) os_retrieve("date"); 
	A(date_r != NULL);
	A(date_r[0] == 9);
	A(date_r[1] == 4);
	A(date_r[2] == 1997);

	complex c = {10, STR, 3L};
	SC(os_store("complex", (void*)&c, sizeof(complex)));
	complex* r = (complex*)  os_retrieve("complex");
	A(r != NULL);
	A(r->num == 10);
	A(r->lnum == 3L);
	A(strcmp(r->str, STR) == 0);

	os_disconnect();
	return EXIT_SUCCESS;
}
