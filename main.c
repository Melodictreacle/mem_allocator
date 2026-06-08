#include <stdio.h>
#include <string.h>
#include <stdlib.h> // For exit
#include "alt_mem.h"

#define LARGE_ALLOC_SIZE (200 * 1024)

// Helper to check if memory is zeroed
void check_zero(void *ptr, size_t size) {
    unsigned char *byte_ptr = (unsigned char *)ptr;
    for (size_t i = 0; i < size; ++i) {
        if (byte_ptr[i] != 0) {
            fprintf(stderr, "Error: Memory not zeroed at byte %zu!\n", i);
            exit(EXIT_FAILURE); // Abort test
        }
    }
     printf(" -> Memory check: OK (all bytes zero)\n");
}


int main(void) {
    printf("Starting memory allocator test...\n");

    printf("\n[Test 1: calloc Small (sbrk)]\n");
    int *c1 = alt_calloc(10, sizeof(int)); // 40 bytes aligned to 40
    printf("c1 (%p) calloc 10 ints\n", c1);
    if(c1) check_zero(c1, 10 * sizeof(int));
    alt_free(c1);

    printf("\n[Test 2: calloc Large (mmap)]\n");
    long *c2 = alt_calloc(50000, sizeof(long)); // > MMAP_THRESHOLD
    printf("c2 (%p) calloc 50000 longs (large)\n", c2);
     if(c2) check_zero(c2, 50000 * sizeof(long)); // mmap should zero it
    alt_free(c2); // Should use munmap

    // Add other malloc/free tests here if desired

    printf("\nMemory allocator test finished.\n");
    return 0;
}