#include <stdlib.h>
#include <stdio.h>
#include "mymalloc.h"

#define SIZE 8192

/* Global variable for storing the beginning of the free list */
void* malloc_h = NULL;

typedef struct flist  {
	int size;
	struct flist *flink;
	struct flist *blink;
} *Flist;

void* allocate(size_t size);
Flist allocateBlock(Flist f, size_t size);
void* findFreeChunk(size_t size);
void freeListAppend(void* ptr, Flist fc);
void freeListPrepend(void* ptr, Flist fc);
void freeListDelete(Flist f);
size_t alignBy8(size_t size);

int main(int argc, char **argv){
	int i;
	size_t size;
	int* a[342];
	

	i = 0;
	size = atoi(argv[1]);
	while(i < 341){
		printf("i: %d\n", i);
		a[i] = my_malloc(size);
		i++;
	}
	a[i] = my_malloc(8192*2);
	for(i = 0; i < 342; i++){
		my_free(a[i]);
		printf("Done with free %d\n", i);
	}
	printf("Free list begin is %#x\n", free_list_begin());
	return 0;
}

void *my_malloc(size_t size){
	Flist f;
	Flist a;

	size = alignBy8(size);
	size += 8;
	printf("Size is %d\n", size);

	/* Case: no free memory chunks in free list */
	if(malloc_h == NULL){
		malloc_h = allocate(size);
		printf("Top of heap is %#x\n", malloc_h);
		f = (Flist) malloc_h;
		f->flink = f;
		f->blink = f;
		a = allocateBlock(f, size);
		printf("TEMP - a is of size %d and is at location %#x\n", a->size, a);
		printf("But returning %#x\n\n", ((void*) a)+8);
		return ((void*) a)+8;
	}
	else{
		printf("Looking for memory of size %d...\n", size);
		f = (Flist) findFreeChunk(size);
		if(f != NULL){
			// printf("TEMP - Memory chunk found of size %d at location %#x\n", f->size, f);
			a = allocateBlock(f, size);
			printf("TEMP - a is of size %d and is at location %#x\n", a->size, a);
		printf("But returning %#x\n\n", ((void*) a)+8);
			return ((void*) a)+8;
		}
	}


}

void my_free(void *ptr){
	Flist f;
	Flist fc;
	printf("Pointer was %#x", ptr);
	ptr = ptr - 8;
	printf(" but is now %#x\n", ptr);

	/* Case: Free list was empty */
	if(malloc_h == NULL){
		printf("TEMP - No free list entries; entry now sole free list entry\n");
		malloc_h = ptr;
		f = (Flist) ptr;
		f->flink = f;
		f->blink = f;
		return;
	}

	fc = (Flist) malloc_h;

	/* Case: One entry in the free list */
	if(fc->blink == fc){
		if((void*) fc < ptr){
			freeListAppend(ptr, fc);
		}
		else{
			freeListPrepend(ptr, fc);
			malloc_h = ptr;
		}
		return;
	}

	/* Case: More than one entry in free list */
	if(ptr < malloc_h){
		freeListPrepend(ptr, fc);
		malloc_h = ptr;
		return;
	}

	fc = fc->blink;		/* Go to the end of the free list */
	printf("fc is 0x%x fc->flink is 0x%x ptr is 0x%x malloc_h is 0x%x\n", fc, fc->flink, ptr, malloc_h);
	while(fc != NULL){
		printf("fc is %#x\n", fc);
		if((((void*) fc) < ptr) && (((void*) fc->flink) > ptr || ((void*) fc->flink == malloc_h))){
			printf("TEMP - Found location where ptr belongs\n");
			printf("TEMP - %#x < %#x < %#x\n", (unsigned int) fc->blink, (unsigned int) ptr, (unsigned int) fc->flink);
			freeListAppend(ptr, fc);
			break;
		}
		fc = fc->blink;
	}
}
		

void *free_list_begin(){
	/* Case: No free memory chunks on free list */
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

void coalesce_free_list(){
	// TODO Write coalesce
}

/* findFreeChunk looks for a chunk of free memory of at least size `size`+8
 * If no such chunk can be found, it returns NULL */
void* findFreeChunk(size_t size){
	Flist f;
	f = (Flist) free_list_begin();
	if(f == NULL){
		fprintf(stderr, "ERROR: findFreeChunk - Called findFreeChunk while free list was empty\n");
		exit(1);
	}

	do {
		if(f->size >= size){
			printf("Found memory!\n");
			return (void*) f;
		}
		else{
			f = (Flist) free_list_next((void*) f);
		}
	} while(f != free_list_begin());
	printf("Failed\n");
	return NULL;

}

void* allocate(size_t size){
	void* h;
	Flist f;
	if(size > (SIZE-16)){
		printf("TEMP - Called sbrk(%d)\n", size);
		h = (void*) sbrk(size);
		f = (Flist) h;
		f->size = size;
	}
	else{
		printf("TEMP - Called sbrk(%d)\n", SIZE);
		h = (void*) sbrk(SIZE);
		f = (Flist) h;
		f->size = SIZE;
	}
	return h;
}

size_t alignBy8(size_t size){
	size += (-size) & (7);
	return size;
}

void freeListAppend(void* ptr, Flist fc){
	Flist f;
	f = (Flist) ptr;

	f->flink = fc->flink;
	f->blink = fc->flink->blink;
	fc->flink->blink = ptr;
	fc->flink = ptr;
}

void freeListPrepend(void* ptr, Flist fc){
	Flist f;
	f = (Flist) ptr;

	f->flink = fc;
	f->blink = fc->blink;
	fc->blink->flink = ptr;
	fc->blink = ptr;
}

void freeListDelete(Flist f){
	if(f->flink == f){
		malloc_h = NULL;
	}
	else{
		f->flink->blink = f->blink;
		f->blink->flink = f->blink;
	}
}

Flist allocateBlock(Flist f, size_t size){
	Flist a;
	if((f->size - size) < 12){
		printf("Size is %d and f->size is %d\n", size, f->size);
		printf("Giving it all\n");
		a = f;
		a->size = f->size;
		printf("TEMP - Removed entry from freelist at location %#x\n", f);
		freeListDelete(f);
		if(malloc_h == NULL) printf("Malloc_h is NULL\n");
		printf("Done\n");
	}
	else{
		a = (Flist) (((void*) f) + (f->size - size));
		a->size = size;
		f->size -= size;
	}
	return a;
}
