#include <stdlib.h>
#include <stdio.h>
#include "mymalloc.h"

#define SIZE 8192

#define DBG 0

/* Global variable for storing the beginning of the free list */
void* malloc_h = NULL;

typedef struct flist  {
	int size;
	struct flist *flink;
	struct flist *blink;
} *Flist;

void* allocateFreshChunk(size_t size);
Flist allocateBlock(Flist f, size_t size);
void* findFreeChunk(size_t size);
void freeListAppend(void* ptr, Flist fc);
void freeListPrepend(void* ptr, Flist fc);
void freeListDelete(Flist f);
size_t alignBy8(size_t size);

/*
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
   */

void *my_malloc(size_t size){
	Flist f;
	Flist a;
	void *ptr;

	size = alignBy8(size); /* Round up to the nearest multiple of 8 */
	size += 8; /* Account for 8 bytes of book keeping */

	/* Case: no free memory chunks in free list */
	if(malloc_h == NULL){
		malloc_h = allocateFreshChunk(size);
		if(DBG == 1) printf("malloc_h is %#x\n", malloc_h);
		if(DBG == 1) printf("TEMP - Top of heap is %#x\n", malloc_h);
		f = (Flist) malloc_h;
		f->flink = f;
		f->blink = f;
		a = allocateBlock(f, size);
		if(DBG == 1) printf("TEMP: my_malloc -  a is of size %d and is at location %#x\n", a->size, a);
		return ((void*) a)+8;
	}
	else{
		if(DBG == 1) printf("malloc_h is %#x\n", malloc_h);
		f = (Flist) findFreeChunk(size);
		if(f != NULL){
			if(DBG == 1) printf("TEMP - Memory chunk found of size %d at location %#x\n", f->size, f);
			a = allocateBlock(f, size);
			if(DBG == 1) printf("TEMP: my_malloc -  a is of size %d and is at location %#x\n", a->size, a);
			return ((void*) a)+8;
		}
		else{
			// if(DBG == 1) printf("Maybe this case!\n");
			// exit(1);
			/* Allocate a fresh chunk */
			ptr = allocateFreshChunk(size);
			/* Put fresh memory at end of the list */
			if(DBG == 1) printf("PREPEND\n");
			freeListPrepend(ptr, (Flist) malloc_h);
			f = (Flist) ptr;
			/* Carve off what is needed */
			a = allocateBlock((Flist) ptr, size);
			if(DBG == 1) printf("TEMP: my_malloc -  a is of size %d and is at location %#x\n", a->size, a);
			if(DBG == 1) printf("malloc_h is %#x\n", malloc_h);
			return ((void*) a)+8;
		}
	}
}

void my_free(void *ptr){
	Flist f;
	Flist fc;
	ptr = ptr - 8;

	/* Case: Free list was empty */
	if(malloc_h == NULL){
		if(DBG == 1) printf("TEMP: my_free - Only entry when free'd so setting malloc_h\n");
		malloc_h = ptr; /* Set malloc_h and link ptr to itself as sole Flist */
		if(DBG == 1) printf("malloc_h is %#x\n", malloc_h);
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
			if(DBG == 1) printf("TEMP: my_free - Two entries; Prepended when free'd so setting malloc_h\n");
			malloc_h = ptr; /* Update malloc_h */
			if(DBG == 1) printf("malloc_h is %#x\n", malloc_h);
		}
		return;
	}

	/* Case: More than one entry in free list */
	/* If ptr is the smallest address seen yet, set it as malloc_h and
	 * prepend it to the free list */
	if(ptr < malloc_h){
		freeListPrepend(ptr, fc);
			if(DBG == 1) printf("TEMP: my_free - Many entries; Prepended when free'd so setting malloc_h\n");
		malloc_h = ptr;
		if(DBG == 1) printf("malloc_h is %#x\n", malloc_h);
		return;
	}

	/* Otherwise find ptr's proper place */
	fc = fc->blink;		/* Go to the end of the free list */
	if(DBG == 1) printf("TEMP - fc is 0x%x fc->flink is 0x%x fc->blink is 0x%x ptr is 0x%x malloc_h is 0x%x\n", fc, fc->flink, fc->blink, ptr, malloc_h);
	while(fc != NULL){
		if((((void*) fc) < ptr) && (((void*) fc->flink) > ptr || ((void*) fc->flink == malloc_h))){
			if(DBG == 1) printf("TEMP - Found location where ptr belongs\n");
			if(DBG == 1) printf("TEMP - %#x < %#x < %#x\n", (unsigned int) fc->blink, (unsigned int) ptr, (unsigned int) fc->flink);
			freeListAppend(ptr, fc);
			return;
		}
		fc = fc->blink;
	}
}


void *free_list_begin(){
	/* Case: No free memory chunks on free list */
	if(malloc_h == NULL){
		return NULL;
	}
	else{ return (void*) malloc_h; }
}

void *free_list_next(void *node){
	Flist f;
	void* first;
	first = free_list_begin();
	f = (Flist) node;
	if(f->flink != NULL && ((void*) f->flink) != first){
		return (void*) f->flink;
	}
	else{
		return NULL;
	}
}

void coalesce_free_list(){
    void *ptr;
    Flist f, fn;
    int i;
    // TODO Write coalesce
    //   > If two adjacent free list entries are encountered
    //          > Update the top free list entry to depict the fact that the lower is now part of the upper
    //  Else continue traversal
	if(DBG == 1){    
		i = 0;
		for(ptr = free_list_begin(); ptr != NULL; ptr = free_list_next(ptr)){
			printf("%d: %#x\n", i, ptr);
			i++;
		}
	}

    ptr = free_list_begin();
    // Traverse free list
    while(ptr != NULL){
        f = (Flist) ptr;
        if((f->flink != ptr) && (f->flink == (ptr + f->size))){
            fn = (Flist) f->flink;
            f->size += fn->size;
            freeListDelete(fn);
        }
        else{
            ptr = free_list_next(ptr);
        }
    }

	if(DBG == 1){
		i = 0;
		for(ptr = free_list_begin(); ptr != NULL; ptr = free_list_next(ptr)){
			printf("%d: %#x\n", i, ptr);
			i++;
		}
	}
}

/* findFreeChunk looks for a chunk of free memory of at least size `size`+8
 * If no such chunk can be found, it returns NULL */
void* findFreeChunk(size_t size){
	Flist f, old;
	f = (Flist) free_list_begin();
	if(DBG == 1) printf("Free_list_begin is %#x\n", f);
	if(f == NULL){
		fprintf(stderr, "ERROR: findFreeChunk - Called findFreeChunk while free list was empty\n");
		exit(1);
	}

	while(f != NULL){
		if(f->size >= size){
			return (void*) f;
		}
		else{
			old = f;
			f = (Flist) free_list_next((void*) f);
			
			if(f == old){ 
				printf("f is %#x\n", f);
				exit(1);
			}
			if(DBG == 1) printf("Looping f is %#x\n", f);
		}
	}
	if(DBG == 1) printf("None found!\n");
	return NULL;
}

void* allocateFreshChunk(size_t size){
	void* h;
	Flist f;
	if(size > (SIZE-16)){
		if(DBG == 1) printf("TEMP - Called sbrk(%d)\n", size);
		h = (void*) sbrk(size);
		f = (Flist) h;
		f->size = size;
	}
	else{
		if(DBG == 1) printf("TEMP - Called sbrk(%d)\n", SIZE);
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
	f->blink = fc;
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
		f->blink->flink = f->flink;
	}
}

Flist allocateBlock(Flist f, size_t size){
	Flist a;
	if((f->size - size) < 16){
		a = f;
		a->size = f->size;
		freeListDelete(f);
		if(DBG == 1) printf("TEMP - Removed entry %#x from free list\n", f);
	}
	else{
		a = (Flist) (((void*) f) + (f->size - size));
		a->size = size;
		f->size -= size;
	}
	return a;
}
