#include <stdio.h>
#include <string.h>
#include "alt_mem.h"

int main(void) {
    printf("Starting memory allocator test...\n");

    printf("\n[Test 1: Allocate Three Adjacent Blocks]\n");
    char *a = alt_malloc(100); printf("a (%p) alloc 100\n", a);
    char *b = alt_malloc(120); printf("b (%p) alloc 120\n", b);
    char *c = alt_malloc(150); printf("c (%p) alloc 150\n", c);

    printf("\n[Test 2: Free Middle Block - No Coalesce]\n");
    printf("Freeing b (%p)...\n", b);
    alt_free(b); // Should just mark b as free

    printf("\n[Test 3: Free First Block - Coalesce Forward]\n");
    printf("Freeing a (%p)...\n", a);
    alt_free(a); // Should mark a free, then merge a and b

    printf("\n[Test 4: Allocate to Test Forward Coalesce]\n");
    // Original sizes: a=100, b=120. Meta size ~24.
    // Merged free block should be around 100 + 120 + 24 = 244 bytes.
    printf("Allocating d (220 bytes)...\n");
    char *d = alt_malloc(220); // Should fit in the merged a+b block and split
    printf("d (%p) alloc 220. Should reuse merged a+b block.\n", d);
    alt_free(d); // Free it for next test

    printf("\n[Test 5: Free Last Block - Coalesce Backward]\n");
    // State: Merged a+b (free), c (alloc)
    printf("Freeing c (%p)...\n", c);
    alt_free(c); // Should mark c free, then merge (a+b) with c.

    printf("\n[Test 6: Allocate to Test Full Coalesce]\n");
    // Merged block should be ~ 100 + 120 + 150 + 2*meta = 370 + 48 = 418
    printf("Allocating e (400 bytes)...\n");
    char *e = alt_malloc(400); // Should fit in the fully merged block
    printf("e (%p) alloc 400. Should reuse fully merged block.\n", e);

    printf("\n[Test 7: Clean up]\n");
    alt_free(e);
    printf("Freed e.\n");

    printf("\nMemory allocator test finished.\n");
    return 0;
}