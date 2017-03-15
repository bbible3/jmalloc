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

int main(int argc, char **argv){
	size_t size;
	size = atoi(argv[1]);
	my_malloc(size);
	return 0;
}

void *my_malloc(size_t size){
	Flist f;
	if(malloc_h == NULL){
		malloc_h = allocate(size);
		printf("TEMP - Top of heap is %#lx\n", malloc_h);
		f = (Flist) malloc_h;
		f->flink = NULL;
		f->flink = NULL;
		printf("TEMP - Size is %d\n", f->size);
	}
}

void my_free(void *ptr);

void *free_list_begin();

void *free_list_next(void *node);

void coalesce_free_list();

caddr_t allocate(size_t size){
	caddr_t h;
	Flist f;
	if(size > (SIZE-16)){
		printf("TEMP - Called sbrk(%d)\n", size);
		h = (caddr_t) sbrk(size);
		f = (Flist) h;
		f->size = size;
	}
	else{
		printf("TEMP - Called sbrk(%d)\n", SIZE);
		h = (caddr_t) sbrk(SIZE);
		f = (Flist) h;
		f->size = SIZE;
	}
	return h;
}
