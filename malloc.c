/*
 * File: malloc.c
 * --------------
 * This is the simple "bump" allocator from lecture.
 * An allocation request is serviced by tacking on the requested
 * space to the end of the heap thus far. 
 * It does not recycle memory (free is a no-op) so when all the
 * space set aside for the heap is consumed, it will not be able
 * to service any further requests.
 *
 * This code is given here just to show the very simplest of
 * approaches to dynamic allocation. You are to replace this code
 * with your own heap allocator implementation.
 */

#include "malloc.h"
#include "printf.h"
#include <stddef.h> // for NULL
#include "strings.h"

extern int __bss_end__;

#define STACK_START 0x8000000
#define STACK_SIZE  0x1000000
#define STACK_END ((char *)STACK_START - STACK_SIZE)

/*
 * The pool of memory used for the heap starts at the top end of the 
 * data section and extends to the bottom end of the stack.
 * It uses symbol __bss_end__ from memmap to locate data end
 * and calculates stack end assuming a 16MB stack.
 *
 * Global variables for the bump allocator:
 *
 * `heap_end` tracks location of next available address in heap
 * `heap_max` tracks location where heap is exhausted
 * 
 */
static void *heap_end = &__bss_end__;
static void *heap_max = STACK_END;


// Simple macro to round up x to multiple of n.
// The efficient but tricky bitwise approach it uses
// works only if n is a power of two -- why?
#define roundup(x,n) (((x)+((n)-1))&(~((n)-1)))

void *malloc (size_t nbytes)
{
    // TODO: replace with your code
    nbytes = roundup(nbytes, 8);
    if ((char *)heap_end + nbytes > (char *)heap_max)
        return NULL;
    void *alloc = heap_end;
    heap_end = (char *)heap_end + nbytes;
    return alloc;
}

void free (void *ptr)
{
    // TODO: fill in your own code here
}

void *realloc (void *orig_ptr, size_t new_size)
{
    // TODO: replace with yorur code
    void *new_ptr = malloc(new_size);
    if (!new_ptr) return NULL;
    // ideally would copy the min of new_size and old_size, but this allocator
    // doesn't know the old_size. Why not? 
    // Why is it "safe" (but not efficient) to copy new_size bytes?
    memcpy(new_ptr, orig_ptr, new_size);
    free(orig_ptr);
    return new_ptr;
}

void heap_dump (const char *label)
{
    printf("\n---------- HEAP DUMP (%s) ----------\n", label);
    // TODO: fill in your own code here
    printf("----------  END DUMP (%s) ----------\n", label);
}

void memory_report (void)
{
    printf("\n=============================================\n");
    printf(  "         Mini-Valgrind Memory Report         \n");
    printf(  "=============================================\n");
    // TODO: fill in for extension
}
