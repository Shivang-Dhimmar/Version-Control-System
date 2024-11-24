CC = gcc

all: folder

folder: folder_management.c 
	$(CC) folder_management.c ../file_operations/mydiff.o ../file_operations/myrevert.o ../file_operations/common.o -o ../../mygit -lz -lcrypto -lssl

clean:
	rm ../../mygit