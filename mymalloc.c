#include <stdlib.h>
#include <stdio.h>
#include "mymalloc.h"

#define SIZE 8192

/* Global variable for storing the beginning of the free list */
void* malloc_h = NULL;

/* A struct used for storing free list bookkeeping info */
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

/* my_malloc gives the user a chunk of memory that is of at least `size` bytes, padded
 * to a multiple of 8. It either calls sbrk in the event that no memory chunks allocated
 * on the heap are sufficient for the requested size or carves off the resepective amount
 * of memory from a chunk of memory already allocated on the heap */
void *my_malloc(size_t size){
	Flist f;
	Flist a;
	void *ptr;

	size = alignBy8(size); /* Round up to the nearest multiple of 8 */
	size += 8; /* Account for 8 bytes of book keeping */

	/* Case: no free memory chunks in free list */
	if(malloc_h == NULL){
		malloc_h = allocateFreshChunk(size);
		f = (Flist) malloc_h;
		f->flink = f; /* f is the sole entry on the free list at this point */
		f->blink = f;
		a = allocateBlock(f, size); /* Get a block of memory as requested by the user */
		return ((void*) a)+8;
	}
	/* Case: free memory chunks available on the free list */
	else{
		f = (Flist) findFreeChunk(size); /* Try to find a sufficiently large chunk */
		/* Case: Sufficiently large chunk found */
		if(f != NULL){
			a = allocateBlock(f, size);
			return ((void*) a)+8;
		}
		/* Case: No sufficiently large chunks available */
		else{
			ptr = allocateFreshChunk(size);
			/* Put fresh memory at end of the list */
			freeListPrepend(ptr, (Flist) malloc_h);
			a = allocateBlock((Flist) ptr, size); /* Carve off what is needed */
			return ((void*) a)+8;
		}
	}
}

/* my_free restores a block of memory back to the free list */
void my_free(void *ptr){
	Flist f;
	Flist fc;
	ptr = ptr - 8;

	/* Case: Free list was empty */
	if(malloc_h == NULL){
		/* Set malloc_h and make doubly-linked list of sole entry */
		malloc_h = ptr;
		f = (Flist) ptr;
		f->flink = f;
		f->blink = f;
		return;
	}

	fc = (Flist) malloc_h;

	/* Case: One entry in the free list */
	if(fc->blink == fc){
		/* If the free'd memory is of a higher address than the first entry of
		 * the free list, append it to the list after the head node */
		if((void*) fc < ptr){
			freeListAppend(ptr, fc);
		}
		/* Otherwise prepend it before the head node and set it to be the head 
		 * node */
		else{
			freeListPrepend(ptr, fc);
			malloc_h = ptr; /* Update malloc_h */
		}
		return;
	}

	/* Case: More than one entry in free list */

	/* If ptr is the smallest address seen yet, simply set it as malloc_h and
	 * prepend it to the free list */
	if(ptr < malloc_h){
		freeListPrepend(ptr, fc);
		malloc_h = ptr;
		return;
	}

	/* Otherwise find ptr's proper place */
	fc = fc->blink;		/* Go to the end of the free list */
	/* Traverse the free list backwards to find ptr's proper location in the 
	 * free list by comparison of memory address */
	while(fc != NULL){
		/* Check if this is the chunks proper place in the free list based of the
		 * ordering of increasing memory address imposed on the free list */
		if((((void*) fc) < ptr) 
				&& (((void*) fc->flink) > ptr || ((void*) fc->flink == malloc_h))){
			freeListAppend(ptr, fc);
			return;
		}
		fc = fc->blink;
	}
}

/* Return a pointer to the head node of the free list if it's not empty,
 * else return NULL */
void *free_list_begin(){
	if(malloc_h == NULL){
		return NULL;
	}
	else{ return (void*) malloc_h; }
}

/* Return a pointer to the node on the free list after `node` if it `node`
 * is not the last node, else return NULL */
void *free_list_next(void *node){
	Flist f;
	void* first;
	first = free_list_begin();
	f = (Flist) node;
	/* List is circular, so if the first is encountered as the flink,
	 * `node` is at the end of the list */
	if(((void*) f->flink) != first){
		return (void*) f->flink;
	}
	else{
		return NULL;
	}
}

/* coalesce_free_list combines adjacent chunks of memory in the free list
 * (i.e. coalesces them) */
void coalesce_free_list(){
    void *ptr;
    Flist f, fn;

    ptr = free_list_begin();
    /* Traverse free list, combining free list entries if they are adjacent */
    while(ptr != NULL){
        f = (Flist) ptr;
		/* f->flink will be adjacent to f if it is exactly the size of
		 * f away from f itself in memory */
        if((f->flink != ptr) && (f->flink == (ptr + f->size))){
            fn = (Flist) f->flink;
            f->size += fn->size;
            freeListDelete(fn);
        }
		/* If entries aren't adjacent, continue traversal */
        else{
            ptr = free_list_next(ptr);
        }
    }
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

	/* Traverse the free memory list and return a pointer to the memory
	 * chunk with adequate space from which memory can be given to user */
	while(f != NULL){
		if(f->size >= size){
			return (void*) f;
		}
		else{
			f = (Flist) free_list_next((void*) f);
		}
	}

	/* No free chunk had adequate memory, so return NULL */
	return NULL;
}

/* allocatFreshChunk makes the sbrk system call to gain enough memory
 * on the heap for at least `size` bytes. sbrk is only called with a
 * value other than SIZE iff `size` is greater than SIZE-16. sbrk
 * also sets the first four bytes of this newly allocated chunk of
 * memory to the size of the chunk itself */
void* allocateFreshChunk(size_t size){
	void* h;
	Flist f;
	if(size > (SIZE-16)){
		h = (void*) sbrk(size);
		f = (Flist) h;
		f->size = size;
	}
	else{
		h = (void*) sbrk(SIZE);
		f = (Flist) h;
		f->size = SIZE;
	}
	return h;
}

/* alignBy8 pads `size` to be a multiple of 8,
 * rounding up as necessary */
size_t alignBy8(size_t size){
	size += (-size) & (7);
	return size;
}

/* freeListAppend appends ptr after the Flist entry `fc` */
void freeListAppend(void* ptr, Flist fc){
	Flist f;
	f = (Flist) ptr;

	f->flink = fc->flink;
	f->blink = fc;
	fc->flink->blink = ptr;
	fc->flink = ptr;
}

/* freeListPrepend prepends ptr before the Flist entry `fc` */
void freeListPrepend(void* ptr, Flist fc){
	Flist f;
	f = (Flist) ptr;

	f->flink = fc;
	f->blink = fc->blink;
	fc->blink->flink = ptr;
	fc->blink = ptr;
}

/* freeListDelete deletes entry `f` from the free list */
void freeListDelete(Flist f){
	/* If f was the sole entry of the free list, update malloc_h
	 * to reflect the fact that no free memory exists on the heap */
	if(f->flink == f){
		malloc_h = NULL;
	}
	else{
		f->flink->blink = f->blink;
		f->blink->flink = f->flink;
	}
}

/* allocateBlock carves off at least `size` bytes from the end of the
 * free list entry `f`. If there is not enough space said free list entry
 * after the allocation would have been performed, the entire remaining
 * free list entry `f` is given to the user and the `f` is removed from
 * the free list. allocateBlock carves memory off the bottom of `f` */
Flist allocateBlock(Flist f, size_t size){
	Flist a;
	/* If there is not adequate space for another allocation after this
	 * one, simply give the user all the remaining memory in f */
	if((f->size - size) < 16){
		a = f; /* Get a pointer to return */
		a->size = f->size;
		freeListDelete(f); /* Delete f from free list */
	}
	/* Otherwise, carve `size` bytes off the bottom of `f` */
	else{
		a = (Flist) (((void*) f) + (f->size - size)); /* Compute pointer */
		a->size = size;
		f->size -= size;
	}
	return a;
}
