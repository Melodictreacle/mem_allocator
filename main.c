#include <stdio.h>
#include "alt_mem.h" 

int main(void) {
    printf("Starting memory allocator test...\n");

    // Try to allocate some memory 
    void *p1 = alt_malloc(100);
    if (p1 == NULL) {
        printf("Allocation 1 failed (as expected initially).\n");
    }

    // Try to free 
    alt_free(p1);
    printf("Called free (did nothing yet).\n");


    printf("Memory allocator test finished.\n");
    return 0;
}