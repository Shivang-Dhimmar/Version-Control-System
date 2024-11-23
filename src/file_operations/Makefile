CC = gcc
CFLAGES = -Wall
INCLUDE_DIR = -I ../../include/file_operations

all: mydiff myrevert

mydiff: mydiff.c common.o 
	$(CC) $(INCLUDE_DIR) mydiff.c common.o -o mydiff

myrevert: myrevert.c common.o 
	$(CC) $(INCLUDE_DIR) myrevert.c common.o -o myrevert

common.o:common.c 
	$(CC) $(CFLAGES) $(INCLUDE_DIR) -c common.c -o common.o

clean:
	rm -f common.o mydiff myrevert