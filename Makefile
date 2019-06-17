DIRECTORY=src
CFLAGS += -g -std=c99 -pedantic -Wall -Wmissing-field-initializers -D_POSIX_C_SOURCE=200809L
CC = gcc
.PHONY = clean

all: server libobjstore

server: commons commands worker signal
	$(CC) $(CFLAGS) $(DIRECTORY)/$@.c $<.o commands.o worker.o signal.o -o $@.o -lpthread
test: testclient
	-rm testout.log
	touch testout.log
	seq 1 50 | xargs -n1 -P50 -I{} ./testclient.o client{} 1 1>>testout.log
	(seq 1 30 | xargs -n1 -P30 -I{} ./testclient.o client{} 2 1>>testout.log) & 
	(seq 31 50 | xargs -n1 -P20 -I{} ./testclient.o client{} 3 1>>testout.log) &
	wait

testclient: libobjstore 
	$(CC) $(CFLAGS) $(DIRECTORY)/$@.c -Llibs/ -lobjstore -o $@.o

libobjstore: commons 
	$(CC) $(CFLAGS) -c $(DIRECTORY)/$@.c -o $@.o
	-mkdir libs
	ar rvs libs/$@.so $@.o $<.o

commons: 
	$(CC) -c $(CFLAGS) $(DIRECTORY)/$@.c -o $@.o

commands:
	$(CC) -c $(CFLAGS) $(DIRECTORY)/$@.c -o $@.o

worker:
	$(CC) -c $(CFLAGS) $(DIRECTORY)/$@.c -o $@.o

signal:
	$(CC) -c $(CFLAGS) $(DIRECTORY)/$@.c -o $@.o

clean:
	-rm *.o
	-rm libs/*.so
	-rm -rf libs
