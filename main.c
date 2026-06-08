#include <stdio.h>
#include <string.h>
#include "alt_mem.h"

#define LARGE_ALLOC_SIZE (200 * 1024) // Larger than MMAP_THRESHOLD

int main(void) {
    printf("Starting memory allocator test...\n");

    printf("\n[Test 1: Small Allocations (sbrk)]\n");
    void *s1 = alt_malloc(100); printf("s1 (%p) alloc 100\n", s1);
    void *s2 = alt_malloc(200); printf("s2 (%p) alloc 200\n", s2);

    printf("\n[Test 2: Large Allocation (mmap)]\n");
    void *m1 = alt_malloc(LARGE_ALLOC_SIZE);
    printf("m1 (%p) alloc %d (large)\n", m1, LARGE_ALLOC_SIZE);

    printf("\n[Test 3: Another Small Allocation (sbrk)]\n");
    // Should still use sbrk, potentially reusing if s1/s2 freed, or extending heap
    void *s3 = alt_malloc(150); printf("s3 (%p) alloc 150\n", s3);

    printf("\n[Test 4: Freeing Large Allocation]\n");
    printf("Freeing m1 (%p)...\n", m1);
    alt_free(m1); // Should trigger munmap

    printf("\n[Test 5: Freeing Small Allocations]\n");
    printf("Freeing s1 (%p)...\n", s1); alt_free(s1);
    printf("Freeing s2 (%p)...\n", s2); alt_free(s2); // Should coalesce with s1 if adjacent
    printf("Freeing s3 (%p)...\n", s3); alt_free(s3); // Should coalesce if adjacent

    printf("\n[Test 6: Large Allocation Again]\n");
    // Should use mmap again, likely at a different address than before
    void *m2 = alt_malloc(LARGE_ALLOC_SIZE);
    printf("m2 (%p) alloc %d (large)\n", m2, LARGE_ALLOC_SIZE);
    alt_free(m2);


    printf("\nMemory allocator test finished.\n");
    return 0;
}