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

void* allocateFreshChunk(size_t size);
Flist allocateBlock(Flist f, size_t size);
void* findFreeChunk(size_t size);
void freeListAppend(void* ptr, Flist fc);
void freeListPrepend(void* ptr, Flist fc);
void freeListDelete(Flist f);
size_t alignBy8(size_t size);

/*
void double_check_memory(int **ptrs, int *dc, int nptrs, int fl_size)
{
	void *low, *l;
	void *high, *h;
	int *ip, i;
	int nbytes;
	int nfl;

	nbytes = 0;
	printf("Start doublecheck\n");

	for (i = 0; i < nptrs; i++) {
		l = (void *) ptrs[i];
		l -= 8;
		ip = (int *) l;

		if (*ip != dc[i]) {
			printf("Error: pointer number %d the wrong size (%d instead of %d)\n", i, *ip, dc[i]);
			exit(1);
		}
		h = l + *ip;
		if (nbytes == 0 || l < low) low = l;
		if (nbytes == 0 || h > high) high = h;
		nbytes += *ip;
	}
	printf("What?\n");

	nfl = 0;
	for (l = free_list_begin(); l != NULL; l = free_list_next(l)) {
		ip = (int *) l;
		h = l + *ip;
		if (nbytes == 0 || l < low) low = l;
		if (nbytes == 0 || h > high) high = h;
		nbytes += *ip;
		nfl++;
	}
	printf("Donde\n");

	if (nbytes != 8192) {
		printf("Error: Total bytes allocated and on the free list = %d, not 8192\n", nbytes);
		exit(0);
	}
	if (high - low != 8192) {
		printf("Error: Highest address (0x%x) minus lowest (0x%x) does not equal 8192\n", (int) high, (int) low);
		exit(0);
	}

	if (nfl != fl_size && fl_size != -1) {
		printf("Error: %d nodes on the free list -- should be %d\n", nfl, fl_size);
		exit(0);
	}
}

main()
{
	int *ptrs[1];
	int dc[1];

	ptrs[0] = my_malloc(64);
	printf("Done\n");
	dc[0] = 72;
	double_check_memory(ptrs, dc, 1, 1);
	printf("Done 2\n");
	printf("Correct\n");
}
*/

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

	size = alignBy8(size); /* Round up to the nearest multiple of 8 */
	size += 8; /* Account for 8 bytes of book keeping */

	/* Case: no free memory chunks in free list */
	if(malloc_h == NULL){
		malloc_h = allocateFreshChunk(size);
		printf("TEMP - Top of heap is %#x\n", malloc_h);
		f = (Flist) malloc_h;
		f->flink = f;
		f->blink = f;
		a = allocateBlock(f, size);
		printf("TEMP - a is of size %d and is at location %#x\n", a->size, a);
		printf("\n");
		return ((void*) a)+8;
	}
	else{
		f = (Flist) findFreeChunk(size);
		if(f != NULL){
			// printf("TEMP - Memory chunk found of size %d at location %#x\n", f->size, f);
			a = allocateBlock(f, size);
			printf("TEMP - a is of size %d and is at location %#x\n", a->size, a);
			printf("\n");
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
		malloc_h = ptr; /* Set malloc_h and link ptr to itself as sole Flist */
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
			malloc_h = ptr; /* Update malloc_h */
		}
		return;
	}

	/* Case: More than one entry in free list */
	/* If ptr is the smallest address seen yet, set it as malloc_h and
	 * prepend it to the free list */
	if(ptr < malloc_h){
		freeListPrepend(ptr, fc);
		malloc_h = ptr;
		return;
	}

	/* Otherwise find ptr's proper place */
	fc = fc->blink;		/* Go to the end of the free list */
	printf("TEMP - fc is 0x%x fc->flink is 0x%x ptr is 0x%x malloc_h is 0x%x\n", fc, fc->flink, ptr, malloc_h);
	while(fc != NULL){
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
		return NULL;
	}
	else{ return (void*) malloc_h; }
}

void *free_list_next(void *node){
	Flist f;
	f = (Flist) node;
	if(f->flink != NULL && f->flink != free_list_begin()){
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
			return (void*) f;
		}
		else{
			f = (Flist) free_list_next((void*) f);
		}
	} while(f != free_list_begin());
	return NULL;
}

void* allocateFreshChunk(size_t size){
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
		a = f;
		a->size = f->size;
		freeListDelete(f);
		if(malloc_h == NULL) printf("Malloc_h is NULL\n");
	}
	else{
		a = (Flist) (((void*) f) + (f->size - size));
		a->size = size;
		f->size -= size;
	}
	return a;
}
