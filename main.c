#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "alt_mem.h"

static void check_zero(void *ptr, size_t size)
{
    unsigned char *byte_ptr = (unsigned char *)ptr;
    for (size_t i = 0; i < size; ++i) {
        if (byte_ptr[i] != 0) {
            fprintf(stderr, "Error: Memory not zeroed at byte %zu!\n", i);
            exit(EXIT_FAILURE);
        }
    }
    printf(" -> Memory check: OK (all bytes zero)\n");
}

static void run_leak_detector_tests(void)
{

    printf("\nProperly freed sbrk allocations (expect 0 leaks from this group)\n");
    void *a = alt_malloc(64);
    void *b = alt_malloc(128);
    void *c = alt_malloc(256);
    printf("  Allocated: a=%p (64B)  b=%p (128B)  c=%p (256B)\n", a, b, c);
    alt_free(a);
    alt_free(b);
    alt_free(c);
    printf("  Freed all three.\n");

    printf("\nProperly freed mmap allocation (expect 0 leaks from this group)\n");
    void *big = alt_malloc(200 * 1024);
    printf("  Allocated mmap block: %p (200 KiB)\n", big);
    alt_free(big);
    printf("  Freed.\n");

    printf("\ncalloc + realloc, all freed (expect 0 leaks)\n");
    int *arr = alt_calloc(10, sizeof(int));
    printf("  calloc(10, 4) -> %p. Checking zeros... ", arr);
    check_zero(arr, 10 * sizeof(int));
    int *arr2 = alt_realloc(arr, 200 * sizeof(int));
    printf("  realloc to 200 ints -> %p\n", arr2);
    alt_free(arr2);

    printf("\nINTENTIONAL LEAKS-these will appear in the report below\n");

    void *leaked_sbrk = alt_malloc(24);
    printf("  leaked_sbrk  = %p (24 B, sbrk) – NOT freed\n", leaked_sbrk);

    void *leaked_mmap = alt_malloc(128 * 1024);
    printf("  leaked_mmap  = %p (128 KiB, mmap) – NOT freed\n", leaked_mmap);

    void *leaked_calloc = alt_calloc(10, 4);
    printf("  leaked_calloc = %p (40 B, sbrk) – NOT freed\n", leaked_calloc);

   
    printf("\nDouble-free warning test (block should NOT appear as a leak)\n");
    void *df = alt_malloc(32);
    printf("  Allocated df=%p (32 B)\n", df);
    alt_free(df);
    printf("  First free: OK.\n");
    printf("  Second free (expect a STATUS_FREE warning):\n");
    alt_free(df);   // intentional double-free to trigger the warning path 

    printf("(The AltMem leak report will print to stderr when main() returns)\n\n");
}

int main(void)
{
    printf("Starting memory allocator test...\n");

    run_leak_detector_tests();

    printf("main() returning now destructor will fire next.\n");
    return 0;
}
