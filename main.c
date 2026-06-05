#include <stdio.h>
#include <string.h>
#include "alt_mem.h"

int main(void) {
    printf("Starting memory allocator test...\n");

    printf("\n[Test 1: Basic Allocation]\n");
    void *p1 = alt_malloc(200); // Allocate a larger block
    printf("p1 (%p) allocated 200 bytes.\n", p1);
    void *p2 = alt_malloc(50);
    printf("p2 (%p) allocated 50 bytes.\n", p2);

    printf("\n[Test 2: Freeing and Splitting]\n");
    printf("Freeing p1 (%p)...\n", p1);
    alt_free(p1); // Free the 200-byte block

    printf("Allocating p3 (80 bytes)...\n");
    void *p3 = alt_malloc(80); // Should reuse and split the block from p1
    printf("p3 (%p) allocated 80 bytes. Should be same base address as p1.\n", p3);

    printf("\n[Test 3: Using the Remainder]\n");
    printf("Allocating p4 (100 bytes)...\n");
    // The remainder from splitting p1 should be 200 - 80 - sizeof(meta)
    // Let's assume sizeof(meta) is 24 (typical for 64-bit with size_t, enum, 2 pointers)
    // Remainder payload = 200 - 80 - 24 = 96.
    // So, a request for 100 should *not* fit in the remainder, it should use sbrk.
    // Let's request something smaller, like 60.
    void *p4 = alt_malloc(60);
    printf("p4 (%p) allocated 60 bytes. Should reuse the remainder of p1's block.\n", p4);


    printf("\n[Test 4: Clean up]\n");
    alt_free(p2);
    alt_free(p3);
    alt_free(p4);
    printf("Freed p2, p3, p4.\n");


    printf("\nMemory allocator test finished.\n");
    return 0;
}