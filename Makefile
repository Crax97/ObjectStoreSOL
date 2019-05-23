DIRECTORY=src
CFLAGS = -peadantic -Wall -lpthread
CC = gcc
.PHONY = clean

all: server libobjstore testclient
		
server: 
	$(CC) $(CFLAGS) $(DIRECTORY)/$@.c -o $(DIRECTORY)/$@.o

testclient: libobjstore
	$(CC) $(CFLAGS) $(DIRECTORY)/$@.c -lobjstore -o $(DIRECTORY)/$@.o

libobjstore: 
	$(CC) -c $(CFLAGS) $(DIRECTORY)/$@.c -o $(DIRECTORY)/$@.o
	ar rvs libs/$@.so $@.o

clean:
	-rm $(DIRECTORY)/*.o
	-rm $(DIRECTORY)/libs/*.so
	-rm -rf $(DIRECTORY)/libs
