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
#include "backtrace.h"

extern int __bss_end__;

#define STACK_START 0x8000000
#define STACK_SIZE  0x1000000
#define STACK_END ((char *)STACK_START - STACK_SIZE)
#define MAX_DUMP_SIZE 16

// SHOULD ALWAYS BE 8 BYTE ALIGNED
struct header {
	size_t payload_size;
	int status;
	int request;
	frame_t f[3];
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

static int num_alloc = 0;
static int num_frees = 0;
static int num_bytes = 0;

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

// Given a well-formatted header, puts redzones on either side of its payload
void add_redzone(struct header *alloc){
	((uint32_t *)alloc)[HEADER_SIZE / sizeof(uint32_t)] = 0xC0DE;
	((uint32_t *)alloc)[(HEADER_SIZE + sizeof(uint32_t) + alloc->payload_size) / sizeof(uint32_t)] = 0xC0DE;
}

// Ensures that the redzones on the ends of alloc's payload are perserved
int check_redzone(struct header *alloc){
	uint32_t *int_alloc = (uint32_t *)alloc;
	return int_alloc[HEADER_SIZE / sizeof(uint32_t)] + int_alloc[(HEADER_SIZE + alloc->payload_size + sizeof(uint32_t)) / sizeof(uint32_t)] - 2 * 0xC0DE;
}

// Returns the header after alloc
struct header *incr(struct header *alloc){
	uint32_t *temp = (uint32_t *)alloc;
	temp += (HEADER_SIZE + alloc->payload_size + 2 * sizeof(uint32_t)) / sizeof(uint32_t);
	return (struct header *)temp;
}

// Returns a pointer to the data referenced by alloc
void *get_data(struct header *alloc){
	uint32_t *temp = (uint32_t *)alloc;
	temp += HEADER_SIZE / 4 + 1;
	return temp;
}

// Returns the header pointing to the data at ptr
struct header *get_header(void *ptr){
	return (struct header *)((uint32_t *)ptr - 1 - HEADER_SIZE/sizeof(uint32_t));
}

// Prints a Mini-Valgrind Alert
// Should happen whenever you try to free corrupted memory
void memory_alert(struct header *alloc){
	printf("\n=============================================\n");
	printf(  " **********  Mini-Valgrind Alert  ********** \n");
	printf(  "=============================================\n");
	printf("Attempt to free address %p that has damaged redzone(s): [%x] [%x]\n", 
		alloc, 
		((unsigned int *)alloc)[HEADER_SIZE / sizeof(int)], 
		((unsigned int *)alloc)[(HEADER_SIZE + sizeof(int) + alloc->payload_size) / sizeof(int)]);
	printf("Block of size %d bytes, allocated by\n", alloc->request);
    for (int i = 0; i < sizeof(alloc->f) / sizeof(alloc->f[0]); i++)
        printf("#%d 0x%x at %s+%d\n", i, alloc->f[i].resume_addr, alloc->f[i].name, alloc->f[i].resume_offset);
}

// Prints a Mini-Valgrind Report
// Returns an int correrpsonding to the number of lost bytes
int memory_report ()
{
    printf("\n=============================================\n");
    printf(  "         Mini-Valgrind Memory Report         \n");
    printf(  "=============================================\n");
	printf("\n");
	printf("malloc/free: %d	allocs, %d frees, %d bytes allocated\n", num_alloc, num_frees, num_bytes);
	
	printf("\n");
	
	struct header *alloc = (struct header *)heap_start; 
	int byte_total = 0;
	int block_total = 0;
	while(alloc < (struct header *)heap_end) {
		if(check_redzone(alloc) == 0){
			byte_total += alloc->request;
			block_total++;
			printf("%d bytes are lost, allocated by\n", alloc->request);
    		for (int i = 0; i < sizeof(alloc->f) / sizeof(alloc->f[0]); i++)
    		    printf("#%d 0x%x at %s+%d\n", i, alloc->f[i].resume_addr, alloc->f[i].name, alloc->f[i].resume_offset);
			printf("\n");
		}
		alloc = incr(alloc);
	}
	printf("Lost %d total bytes in %d block", byte_total, block_total);	
	return byte_total;
}

/**
* Given a contiguous chunk of free memory of size amt_free immediately following the header alloc, split
* that memory into two pieces: one of size new size and one for whatever remains
* Now also handles redzones
*/
void *split(struct header *alloc, size_t amt_free, size_t new_size){
	if(amt_free >= new_size + HEADER_SIZE + 4 * sizeof(uint32_t)) { // add a new header for the remaining memory
		alloc->payload_size = new_size;
		
		incr(alloc)->payload_size = amt_free - new_size - HEADER_SIZE - 4 * sizeof(uint32_t);
		add_redzone(incr(alloc));
		free(get_data(incr(alloc)));
	} else {
		alloc->payload_size = amt_free - 2 * sizeof(uint32_t);
	}

	alloc->status = 1;
	add_redzone(alloc);
	return get_data(alloc);
}

// Simple macro to round up x to multiple of n.
// The efficient but tricky bitwise approach it uses
// works only if n is a power of two -- why?
#define roundup(x,n) (((x)+((n)-1))&(~((n)-1)))

void *malloc (size_t desired)
{	
	if(desired == 0) return NULL; // Don't need to allocate data

    size_t nbytes = roundup(desired, 8);
	num_bytes += nbytes;
	num_alloc++;

	// Search for an available slot in the existing heap
	struct header *alloc = (struct header *)heap_start;	
	while(alloc < (struct header *)heap_end) {
		if(alloc->status == 0 && alloc->payload_size >= nbytes){
			backtrace(alloc->f, sizeof(alloc->f) / sizeof(alloc->f[0]));
			return split(alloc, alloc->payload_size + 2 * sizeof(uint32_t), nbytes);
		}
		alloc = incr(alloc);
	}	

	// Worst case add a new block at the end
	alloc = (struct header *)sbrk(nbytes + HEADER_SIZE + 2 * sizeof(uint32_t));
	alloc->payload_size = nbytes;
	alloc->status = 1;
	alloc->request = desired;
	backtrace(alloc->f, sizeof(alloc->f) / sizeof(alloc->f[0]));
	add_redzone(alloc);
	return get_data(alloc);
}

void free (void *ptr)
{
	if(ptr == NULL) return; // Otherwise could free the top of memory

	num_frees++;
    struct header *alloc = get_header(ptr);
	alloc->status = 0;

	if(check_redzone(alloc) != 0) { // Don't free mishandled memory
		memory_alert(alloc);
		return;
	}

	// Free all contiguous unused blocks
	struct header *next_alloc = incr(alloc);
	while(next_alloc < (struct header *)heap_end && next_alloc->status == 0){
		alloc->payload_size += HEADER_SIZE + next_alloc->payload_size + 2 * sizeof(uint32_t);
		next_alloc = incr(next_alloc);
	}	
}

void *realloc (void *orig_ptr, size_t desired) 
{
	if(orig_ptr == NULL) return malloc(desired); // Specifically defined behavior
	if(desired == 0) { // Specifically defined behavior
		free(orig_ptr);
		return NULL;
	}

	size_t new_size = roundup(desired, 8);
    struct header *alloc = get_header(orig_ptr);

	// Attempt to resize in place
	int amt_free = -1 * HEADER_SIZE; //should also be in basic
	struct header *next_alloc = alloc;
	next_alloc->status = 0;
	while(next_alloc < (struct header *)heap_end && next_alloc->status == 0){
		amt_free += HEADER_SIZE + next_alloc->payload_size + 2 * sizeof(uint32_t);
		if(amt_free >= new_size + 2 * sizeof(uint32_t)){
			backtrace(alloc->f, sizeof(alloc->f) / sizeof(alloc->f[0]));
			return split(alloc, amt_free, new_size);
		}
		next_alloc = incr(next_alloc);
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
		printf("Redzones [%x] [%x]\n", 
			((unsigned int *)alloc)[HEADER_SIZE / sizeof(int)], 
			((unsigned int *)alloc)[(HEADER_SIZE + sizeof(int) + alloc->payload_size) / sizeof(int)]);
		// Note: The choice to interpret the data as a char* is arbitrary but helps with debugging
		char *data = (char *)get_data(alloc);
		for(int i = 0; i < MAX_DUMP_SIZE; i++){
			printf("%c", data[i]);
		}
		printf("\n");

		count++;
		alloc = incr(alloc);
	}	
    printf("----------  END DUMP (%s) ----------\n", label);
}
