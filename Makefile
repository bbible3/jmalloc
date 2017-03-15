#CS 360 Lab 7 Makefile

CC = gcc 

CFLAGS = -g

EXECUTABLES: mymalloc

all: $(EXECUTABLES)

mymalloc: mymalloc.o
	$(CC) -g -o mymalloc mymalloc.o 

mymalloc.o: mymalloc.c
	$(CC) $(CFLAGS)  -c mymalloc.c

#make clean will rid your directory of the executable,
#object files, and any core dumps you've caused
clean:
	rm mymalloc
	rm mymalloc.o

