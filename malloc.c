/*
 * File: malloc.c
 * --------------
 * This is the simple "bump" allocator from lecture.
 * An allocation request is serviced by using sbrk to extend
 * the heap segment. 
 * It does not recycle memory (free is a no-op) so when all the
 * space set aside for the heap is consumed, it will not be able
 * to service any further requests.
 *
 * This code is given here just to show the very simplest of
 * approaches to dynamic allocation. You are to replace this code
 * with your own heap allocator implementation.
 */

#include "malloc.h"
#include "malloc_internal.h"
#include "printf.h"
#include <stddef.h> // for NULL
#include "strings.h"

extern int __bss_end__;

#define STACK_START 0x8000000
#define STACK_SIZE  0x1000000
#define STACK_END ((char *)STACK_START - STACK_SIZE)
#define MAX_DUMP_SIZE 16

// SHOULD ALWAYS BE 8 BYTE ALIGNED
struct header {
	size_t payload_size;
	int status;
};
#define HEADER_SIZE sizeof(struct header)
/*
 * The pool of memory available for the heap starts at the upper end of the 
 * data section and can extend from there up to the lower end of the stack.
 * It uses symbol __bss_end__ from memmap to locate data end
 * and calculates stack end assuming a 16MB stack.
 *
 * Global variables for the bump allocator:
 *
 * `heap_start`  location where heap segment starts
 * `heap_end`    location at end of in-use portion of heap segment 
 */

// Initial heap segment starts at bss_end and is empty
static void *heap_start = &__bss_end__;
static void *heap_end = &__bss_end__;

void *sbrk(int nbytes)
{
    void *prev_end = heap_end;
    if ((char *)prev_end + nbytes > STACK_END) {
        return NULL;
    } else {
        heap_end = (char *)prev_end + nbytes;
        return prev_end;
    }
}

/**
* Given a contiguous chunk of free memory of size amt_free immediately following the header alloc, split
* that memory into two pieces: one of size new size and one for whatever remains
*/
void *split(struct header *alloc, size_t amt_free, size_t new_size){
	alloc->payload_size = new_size;
	alloc->status = 1;
	if(amt_free - new_size > 0) { // add a new header for the remaining memory
		alloc[(HEADER_SIZE + alloc->payload_size) / HEADER_SIZE].payload_size = amt_free - new_size - HEADER_SIZE;
		free(alloc + (HEADER_SIZE + alloc->payload_size) / HEADER_SIZE);
	}
	return alloc + 1;
}

// Simple macro to round up x to multiple of n.
// The efficient but tricky bitwise approach it uses
// works only if n is a power of two -- why?
#define roundup(x,n) (((x)+((n)-1))&(~((n)-1)))

void *malloc (size_t nbytes)
{	
	if(nbytes == 0) return NULL; // Don't need to allocate data

    nbytes = roundup(nbytes, 8);

	// Search for an available slot in the existing heap
	struct header *alloc = (struct header *)heap_start;
	while(alloc < (struct header *)heap_end) {
		if(alloc->status == 0 && alloc->payload_size >= nbytes)
			return split(alloc, alloc->payload_size, nbytes);
		alloc += (HEADER_SIZE + alloc->payload_size) / HEADER_SIZE;
	}	

	// Worst case add a new block at the end
	alloc = (struct header *)sbrk(nbytes + HEADER_SIZE);
	alloc->payload_size = nbytes;
	alloc->status = 1;
	return alloc + 1;
}

void free (void *ptr)
{
	if(ptr == NULL) return; // Otherwise could free the top of memory

    struct header *alloc = (struct header *)ptr - 1;
	alloc->status = 0;
	
	// Free all contiguous unused blocks
	struct header *next_alloc = alloc + (HEADER_SIZE + alloc->payload_size) / HEADER_SIZE;
	while(next_alloc < (struct header *)heap_end && next_alloc->status == 0){
		alloc->payload_size += HEADER_SIZE + next_alloc->payload_size;
		next_alloc += (HEADER_SIZE + next_alloc->payload_size) / HEADER_SIZE;
	}	
}

void *realloc (void *orig_ptr, size_t new_size) 
{
	if(orig_ptr == NULL) return malloc(new_size); // Specifically defined behavior
	if(new_size == 0) { // Specifically defined behavior
		free(orig_ptr);
		return NULL;
	}

	new_size = roundup(new_size, 8);
    struct header *alloc = (struct header *)orig_ptr - 1;

	// Attempt to resize in place
	size_t amt_free = alloc->payload_size;
	struct header *next_alloc = alloc + (HEADER_SIZE + alloc->payload_size) / HEADER_SIZE;
	while(next_alloc < (struct header *)heap_end && next_alloc->status == 0){
		amt_free += HEADER_SIZE + next_alloc->payload_size;
		if(amt_free >= new_size)
			return split(alloc, amt_free, new_size);
		next_alloc += (HEADER_SIZE + next_alloc->payload_size) / HEADER_SIZE;
	}	

	// Worst case add a new block at the end
    void *new_ptr = malloc(new_size);
    if (!new_ptr) return NULL;
    memcpy(new_ptr, orig_ptr, new_size);
    free(orig_ptr);
    return new_ptr;
}

void heap_dump (const char *label)
{
    printf("\n---------- HEAP DUMP (%s) ----------\n", label);
    printf("Heap segment at %p - %p\n", heap_start, heap_end);
	struct header *alloc = (struct header *)heap_start; 
	int count = 0;
	while(alloc < (struct header *)heap_end) {
		printf("Block #%d Status: %d Size: %d \n", count, alloc->status, alloc->payload_size); 

		// Note: The choice to interpret the data as a char* is arbitrary but helps with debugging
		char *data = (char *)(alloc + 1);
		for(int i = 0; i < MAX_DUMP_SIZE; i++){
			printf("%c", data[i]);
		}
		printf("\n");

		count++;
		alloc += (HEADER_SIZE + alloc->payload_size) / HEADER_SIZE;
	}	
    printf("----------  END DUMP (%s) ----------\n", label);
}

void memory_report (void)
{
    printf("\n=============================================\n");
    printf(  "         Mini-Valgrind Memory Report         \n");
    printf(  "=============================================\n");
    // TODO: fill in for extension
}
