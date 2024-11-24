CC = gcc
CFLAGES = -Wall
INCLUDE_DIR = -I ../../include/file_operations

all: mydiff myrevert
mydiff: mydiff.c common.o 
	$(CC) $(CFLAGES) $(INCLUDE_DIR) -c mydiff.c -o mydiff.o

myrevert: myrevert.c common.o 
	$(CC) $(CFLAGES) $(INCLUDE_DIR) -c myrevert.c -o myrevert.o

common.o:common.c 
	$(CC) $(CFLAGES) $(INCLUDE_DIR) -c common.c -o common.o

clean:
	rm -f common.o mydiff myrevert