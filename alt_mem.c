#include "alt_mem.h" 

#include <stdio.h>      // For perror
#include <stdlib.h>     // For exit, EXIT_FAILURE
#include <unistd.h>     // For sbrk, sysconf, getpagesize
#include <sys/mman.h>   // For mmap, munmap
#include <string.h>     // For memset, memcpy


// Alignment: Most CPUs access memory more efficiently if data starts at
// addresses that are multiples of 4 or 8. We'll align to 8 bytes.
// This macro takes a size 'x' and rounds it UP to the nearest multiple of 8.
// Example: ALIGN8(3) -> 8, ALIGN8(8) -> 8, ALIGN8(10) -> 16
#define ALIGN8(x) (((x) + 7) & (~7))

// Threshold for using mmap instead of sbrk. Allocations larger than or
// equal to this size will use mmap. 128 KiB is a common threshold.
#define MMAP_THRESHOLD (128 * 1024)

// Error Handling Helper 
// A simple macro to check for errors and exit if something critical fails.
#define DIE(assertion, call_description)                                \
    do {                                                                \
        if (assertion) {                                                \
            perror(call_description);                                   \
            exit(EXIT_FAILURE);                                         \
        }                                                               \
    } while (0)



void *alt_malloc(size_t size) {
    (void)size; // Prevent unused variable warning for now
    fprintf(stderr, "alt_malloc not implemented yet!\n");
    return NULL;
}

void alt_free(void *ptr) {
    (void)ptr; // Prevent unused variable warning
    fprintf(stderr, "alt_free not implemented yet!\n");
}

void *alt_calloc(size_t nmemb, size_t size) {
    (void)nmemb; (void)size; // Prevent unused variable warnings
    fprintf(stderr, "alt_calloc not implemented yet!\n");
    return NULL;
}

void *alt_realloc(void *ptr, size_t size) {
    (void)ptr; (void)size; // Prevent unused variable warnings
    fprintf(stderr, "alt_realloc not implemented yet!\n");
    return NULL;
}