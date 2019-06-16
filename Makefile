DIRECTORY=src
CFLAGS += -g -std=c99 -pedantic -Wall -Wmissing-field-initializers
CC = gcc
.PHONY = clean

all: server libobjstore

server: commons
	$(CC) $(CFLAGS) $(DIRECTORY)/$@.c $<.o -o $@.o -lpthread
test: testclient
	for ((i=1; i<=50; i++)); do \
		./testclient.o client$$i 1 & \
	done

testclient: libobjstore 
	$(CC) $(CFLAGS) $(DIRECTORY)/$@.c -Llibs/ -lobjstore -o $@.o

libobjstore: commons 
	$(CC) $(CFLAGS) -c $(DIRECTORY)/$@.c -o $@.o
	-mkdir libs
	ar rvs libs/$@.so $@.o $<.o

commons: 
	$(CC) -c $(CFLAGS) $(DIRECTORY)/$@.c -o $@.o

clean:
	-rm *.o
	-rm libs/*.so
	-rm -rf libs
