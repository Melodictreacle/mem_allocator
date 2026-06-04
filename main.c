#include <stdio.h>
#include <string.h> // For strcpy
#include "alt_mem.h"

int main(void) {
    printf("Starting memory allocator test...\n");

    printf("Attempting allocation 1 (100 bytes)...\n");
    char *p1 = alt_malloc(100);
    if (p1) {
        printf(" -> Allocation 1 successful! Address: %p\n", p1);
        // Try writing to the allocated memory
        strcpy(p1, "Hello");
        printf(" -> Wrote 'Hello' to p1.\n");
        // Access the string through the pointer
        printf(" -> p1 contains: %s\n", p1);
    } else {
        printf(" -> Allocation 1 failed.\n");
    }

    printf("\nAttempting allocation 2 (50 bytes)...\n");
    int *p2 = alt_malloc(50); // Allocate space for about 12 ints
     if (p2) {
        printf(" -> Allocation 2 successful! Address: %p\n", p2);
        // Try writing some integers
        for (int i = 0; i < 10; ++i) {
            p2[i] = i * 10;
        }
        printf(" -> Wrote 10 integers to p2.\n");
        for(int i=0;i<10;i++)
         printf(" -> p2[%d] contains: %d\n",i,p2[i]);
    } else {
        printf(" -> Allocation 2 failed.\n");
    }

    // We still can't free memory properly yet!
    // alt_free(p1);
    // alt_free(p2);

    printf("\nMemory allocator test finished.\n");
    return 0;
}