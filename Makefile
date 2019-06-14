DIRECTORY=src
CFLAGS += -g -pedantic -Wall -lpthread
CC = gcc
.PHONY = clean

all: server libobjstore testclient
		
server: commons
	$(CC) $(CFLAGS) $(DIRECTORY)/$@.c $<.o -o $@.o

testclient: libobjstore commons
	$(CC) $(CFLAGS) $(DIRECTORY)/$@.c commons.o -Llibs/ -lobjstore -o $@.o

libobjstore: commons 
	$(CC) -c $(CFLAGS) $(DIRECTORY)/$@.c $<.o -o $@.o
	-mkdir libs
	ar rvs libs/$@.so $@.o

commons: 
	$(CC) -c $(CFLAGS) $(DIRECTORY)/$@.c -o $@.o

clean:
	-rm *.o
	-rm libs/*.so
	-rm -rf libs
