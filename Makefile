OPTS= -g
CFLAGS += $(OPTS) -std=c99 -pedantic -Wall -Wmissing-field-initializers -D_POSIX_C_SOURCE=200809L
CC = gcc
.PHONY = clean

all: server libobjstore testclient

server: src/commons.c src/commands.c src/worker.c src/signal.c
	$(CC) $(CFLAGS) src/$@.c $? -o $@ -lpthread

test: testclient
	printf '' > testout.log
	seq 1 50 | xargs -n1 -P50 -I{} ./testclient client{} 1 1>>testout.log
	(seq 1 30 | xargs -n1 -P30 -I{} ./testclient client{} 2 1>>testout.log) & 
	(seq 31 50 | xargs -n1 -P20 -I{} ./testclient client{} 3 1>>testout.log) &
	

testclient: libobjstore 
	$(CC) $(CFLAGS) src/$@.c -Llibs/ -lobjstore -o $@

libobjstore: src/commons.c
	$(CC) $(CFLAGS) -c src/$@.c $< 
	mkdir -p libs
	ar rvs libs/$@.so $@.o commons.o
	
clean:
	-rm *.o
	-rm libs/*.so
	-rm -rf libs
	-rm server
	-rm testclient
