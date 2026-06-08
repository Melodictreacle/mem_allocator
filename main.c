#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "alt_mem.h"

// check_zero helper from before...
void check_zero(void *ptr, size_t size) { /* ... */ }

int main(void) {
    printf("Starting memory allocator test...\n");

    printf("\n[Test 1: realloc ptr=NULL]\n");
    void *r1 = alt_realloc(NULL, 100);
    printf("r1 (%p) realloc(NULL, 100) (like malloc)\n", r1);

    printf("\n[Test 2: realloc size=0]\n");
    printf("Realloc r1 (%p) to size 0 (like free)...\n", r1);
    void *r_null = alt_realloc(r1, 0);
    if (r_null == NULL) printf(" -> OK, returned NULL\n"); else printf(" -> FAIL, did not return NULL\n");
    // r1 is now freed (or should be)

    printf("\n[Test 3: realloc - Shrinking with Split]\n");
    char *r2 = alt_malloc(500);
    strcpy(r2, "Test string data");
    printf("r2 (%p) alloc 500. Contains: '%s'\n", r2, r2);
    printf("Realloc r2 to 100 bytes...\n");
    char *r2_shrunk = alt_realloc(r2, 100);
    printf("r2_shrunk (%p) realloc to 100. Should be same addr as r2.\n", r2_shrunk);
    printf(" -> Contains: '%.*s'\n", 100, r2_shrunk); // Print only 100 chars
    // Now there should be a free block of ~376 bytes after r2_shrunk

    printf("\n[Test 4: realloc - Growing using adjacent free block]\n");
    printf("Realloc r2_shrunk back to 400 bytes...\n");
    char *r2_grown = alt_realloc(r2_shrunk, 400);
    printf("r2_grown (%p) realloc to 400. Should be same addr as r2/r2_shrunk.\n", r2_grown);
    printf(" -> Original string still there? '%.*s'\n", 20, r2_grown);
    // Should have merged with the free block created in Test 3

    printf("\n[Test 5: realloc - Growing by moving]\n");
    char *r3 = alt_malloc(50); // Allocate a small block right after r2_grown potentially
    printf("r3 (%p) alloc 50.\n", r3);
    printf("Realloc r2_grown to 1000 bytes...\n");
    char *r2_moved = alt_realloc(r2_grown, 1000);
    printf("r2_moved (%p) realloc to 1000. Should be at a NEW address.\n", r2_moved);
    printf(" -> Original string still there? '%.*s'\n", 20, r2_moved);

    printf("\n[Test 6: Clean up]\n");
    alt_free(r3);
    alt_free(r2_moved); // Free the final location of the block
    printf("Freed r3, r2_moved.\n");


    printf("\n[Test 7: realloc mmap block]\n");
    size_t large_size = 200 * 1024;
    size_t larger_size = 300 * 1024;
    char *m1 = alt_malloc(large_size);
    printf("m1 (%p) alloc %zu (mmap)\n", m1, large_size);
    memset(m1, 'A', large_size); // Fill with 'A'
    printf("Realloc m1 to %zu bytes...\n", larger_size);
    char *m2 = alt_realloc(m1, larger_size);
    printf("m2 (%p) realloc to %zu. Should be NEW address.\n", m2, larger_size);
    if (m2 && m2[0] == 'A' && m2[large_size - 1] == 'A') {
        printf(" -> Data seems to be copied correctly.\n");
    } else {
         printf(" -> Data copy check failed!\n");
    }
    alt_free(m2);


    printf("\nMemory allocator test finished.\n");
    return 0;
}