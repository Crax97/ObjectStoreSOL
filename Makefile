DIRECTORY=src
CFLAGS += -pedantic -Wall -lpthread
CC = gcc
.PHONY = clean

all: server libobjstore testclient
		
server: commons
	$(CC) $(CFLAGS) $(DIRECTORY)/$@.c $<.o -o $@.o

testclient: libobjstore
	$(CC) $(CFLAGS) $(DIRECTORY)/$@.c -Llibs/ -lobjstore -o $@.o

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
