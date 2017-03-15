#include <stdlib.h>
#include <stdio.h>
#include "mymalloc.h"

#define SIZE 8192

/* Global variable for storing the beginning of the free list */
caddr_t malloc_h = NULL;

typedef struct flist  {
	int size;
	struct flist *flink;
	struct flist *blink;
} *Flist;

caddr_t allocate(size_t size);
caddr_t findFreeChunk(size_t size);
size_t alignBy8(size_t size);

int main(int argc, char **argv){
	size_t size;
	size = atoi(argv[1]);
	my_malloc(size);
	return 0;
}

void *my_malloc(size_t size){
	Flist f;

	size = alignBy8(size);

	if(malloc_h == NULL){
		malloc_h = allocate(size);
		printf("TEMP - Top of heap is %#lx\n", malloc_h);
		f = (Flist) malloc_h;
		f->flink = NULL;
		f->flink = NULL;
		printf("TEMP - Size is %d\n", f->size);
	}
	printf("Looking for memory...\n");
	f = (Flist) findFreeChunk(size);
	if(f != NULL){
	printf("TEMP - Memory chunk found of size %d\n", f->size);
	}
}

void my_free(void *ptr);

void *free_list_begin(){
	if(malloc_h == NULL){
		printf("TEMP - No free list entries\n");
		return NULL;
	}
	else{ return (void*) malloc_h; }
}

void *free_list_next(void *node){
	Flist f;
	f = (Flist) node;
	if(f->flink != NULL){
		return (void*) f->flink;
	}
	else{
		return NULL;
	}
}

void coalesce_free_list();

caddr_t findFreeChunk(size_t size){
	Flist f;
	f = (Flist) free_list_begin();

	while(f != NULL){
		if(f->size >= size+8){
			printf("Found memory!\n");
			return (caddr_t) f;
		}
		else{
			f = (Flist) free_list_next((void*) f);
		}
	}
	return NULL;

}

caddr_t allocate(size_t size){
	caddr_t h;
	Flist f;
	if(size > (SIZE-16)){
		printf("TEMP - Called sbrk(%d)\n", size+8);
		h = (caddr_t) sbrk(size+8);
		f = (Flist) h;
		f->size = size+8;
	}
	else{
		printf("TEMP - Called sbrk(%d)\n", SIZE);
		h = (caddr_t) sbrk(SIZE);
		f = (Flist) h;
		f->size = SIZE;
	}
	return h;
}

size_t alignBy8(size_t size){
	size += (-size) & (7);
	return size;
}

/*
 8: 01000
    10111
	00111


 9: 01001 
    10110
	00110

	01001
  + 00110
  -------
    01111

 5: 00101
    11010
	00010

	00101
  + 00010
  -------
    00111

16: 10000
*/
