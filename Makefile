DIRECTORY=src
CFLAGS += -g -std=c99 -pedantic -Wall -Wmissing-field-initializers
CC = gcc
.PHONY = clean

all: server libobjstore testclient
		
server: commons
	$(CC) $(CFLAGS) $(DIRECTORY)/$@.c $<.o -o $@.o -lpthread

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
