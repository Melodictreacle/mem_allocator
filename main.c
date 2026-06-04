#include "alt_mem.h"
#include <stdio.h>
#include <string.h>

int main(void) {
  printf("Starting memory allocator test....\n");

  printf("Attempting allocation 1 (100 bytes)...\n");
  char *p1 = alt_malloc(100);
  if (p1) { /* */
  } else {  /* */
  }
  printf("\n Attempting allocation 2 (50 bytes)\n");
  char *p2 = alt_malloc(50);
  if (p2) { /* */
  } else {  /* */
  }

  printf("Freeing allocation 2 \n");
  alt_free(p2);

  printf("\n Attempting allocation 3 (20 bytes) after freeing \n");
  void *p3 = alt_malloc(20);
  if (p3) {
    printf("Allocation 3 successful Address :%p\n", p3);
  } else {
    printf("Allocation 3 failed\n");
  }

  printf("\nMemory Allocator test finished\n");
  return 0;
}