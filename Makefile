DIRECTORY=src
CFLAGS = -pedantic -Wall -lpthread
CC = gcc
.PHONY = clean

all: server libobjstore testclient
		
server: 
	$(CC) $(CFLAGS) $(DIRECTORY)/$@.c -o $@.o

testclient: libobjstore
	$(CC) $(CFLAGS) $(DIRECTORY)/$@.c -Llibs/ -lobjstore -o $@.o

libobjstore: 
	$(CC) -c $(CFLAGS) $(DIRECTORY)/$@.c -o $@.o
	-mkdir libs
	ar rvs libs/$@.so $@.o

clean:
	-rm *.o
	-rm libs/*.so
	-rm -rf libs
