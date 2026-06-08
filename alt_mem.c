#include "alt_mem.h"

#include <stdbool.h>  // For bool type
#include <stddef.h>
#include <stdio.h>    // For perror
#include <stdlib.h>   // For exit, EXIT_FAILURE
#include <string.h>   // For memset, memcpy
#include <sys/mman.h> // For mmap, munmap
#include <unistd.h>   // For sbrk, sysconf, getpagesize

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
#define DIE(assertion, call_description)                                       \
  do {                                                                         \
    if (assertion) {                                                           \
      perror(call_description);                                                \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
  } while (0)

// Data Structures

// Possible states for a memory block
typedef enum {
  STATUS_FREE,  // Block is free and available for allocation
  STATUS_ALLOC, // Block is currently allocated (used by sbrk)
  STATUS_MAPPED // Block was allocated using mmap
} block_status;

// Metadata structure prepended to each memory block
typedef struct block_meta {
  size_t size; // Size of the *user* data area (payload), doesn't include
               // sizeof(struct block_meta)
  block_status status;     // Current status of the block (FREE, ALLOC, MAPPED)
  struct block_meta *next; // Pointer to the next block_meta in the list
  struct block_meta *prev; // Pointer to the previous block_meta in the list
} block_meta_t;

// Global Variables

// Head of the linked list of memory blocks. Initially NULL.
static block_meta_t *head = NULL;

// Flag to track if we've done the initial large allocation
static bool preallocated = false;

// Helper functions

// Helper function to request memory from the OS using sbrk
// Returns a pointer to the new block's metadata, or NULL on failure
static block_meta_t *request_space(size_t size, block_meta_t *last_entry) {
  // Request enough space for the metadata block PLUS the requested user size
  block_meta_t *block = sbrk(sizeof(block_meta_t) + size);
  if (block == (void *)-1) { // sbrk returns (void*)-1 on error
    perror("sbrk failed");
    return NULL; // Indicate failure
  }

  // Initialize the metadata for this new block
  block->size = size;
  block->status = STATUS_ALLOC; // Mark as allocated
  block->next = NULL;           // This is the new last block
  block->prev = last_entry;     // Link to the previous block

  // Update the previous block's 'next' pointer if it exists
  if (last_entry) {
    last_entry->next = block;
  } else {
    // If there was no previous block, this is the new head of the list
    head = block;
  }

  return block;
}

// Helper function to get the metadata block from a user pointer
//  Returns NULL if ptr is invalid
static block_meta_t *get_block_ptr(void *ptr) {
  if (!ptr) {
    return NULL;
  }
  // The metadata block is located just BEFORE the user pointer
  return (block_meta_t *)ptr - 1;
}

// Helper function to find the last block in our linked list
// Needed to link a new sbrk block at the end
static block_meta_t *find_last_entry(void) {
  block_meta_t *current = head;
  if (!current) {
    return NULL; // List is empty
  }
  // Traverse the list until we find the block whose 'next' is NULL
  while (current->next) {
    current = current->next;
  }
  return current;
}

// Find the best-fit block for a given size
//  Returns the best block found , or NULL if no suitable block exists

static block_meta_t *find_best_fit(size_t size) {
  block_meta_t *current = head;
  block_meta_t *best = NULL;

  while (current) {
    if (current->status == STATUS_FREE && current->size >= size) {
      if (!best || current->size < best->size) {
        best = current;
      }
      if (best->size == size) {
        break;
      }
    }
    current = current->next;
  }

  return best;
}

// Coalesce adjacent free blocks starting from 'block'
// Merges with next block(s) if free and adjacent.
// Merges with previous block(s) if free and adjacent.
static void join_free_blocks(block_meta_t *block) {
    printf("DEBUG: Attempting to coalesce around block %p (size %zu)\n", (void*)block, block->size);

    // Coalesce with the NEXT block (if possible)
    if (block->next && block->next->status == STATUS_FREE) {
        // Check for physical adjacency
        if ((char *)block + sizeof(block_meta_t) + block->size == (char *)block->next) {
            printf("DEBUG: Coalescing block %p with next block %p\n", (void*)block, (void*)block->next);
            // Increase current block size
            block->size += sizeof(block_meta_t) + block->next->size;
            // Bypass the merged block in the list
            block->next = block->next->next;
            if (block->next) {
                block->next->prev = block;
            }
        } else {
             printf("DEBUG: Next block %p is FREE but not adjacent.\n", (void*)block->next);
        }
    }

    // Coalesce with the PREVIOUS block (if possible)
    //    Note: If we merged with the next block above, 'block' itself is still the
    //    correct starting point for checking the previous block.
    if (block->prev && block->prev->status == STATUS_FREE) {
        // Check for physical adjacency
        if ((char *)block->prev + sizeof(block_meta_t) + block->prev->size == (char *)block) {
             printf("DEBUG: Coalescing block %p with previous block %p\n", (void*)block, (void*)block->prev);
            // Increase previous block's size
            block->prev->size += sizeof(block_meta_t) + block->size;
            // Bypass the current block ('block') in the list
            block->prev->next = block->next;
            if (block->next) {
                block->next->prev = block->prev;
            }
            // Important: After merging with previous, the 'block' pointer is now
            // invalid in the sense that the 'previous' block is the one we should
            // consider as the result of the merge. However, since we're just
            // finishing the free operation, we don't need to update the caller's
            // 'block' variable. The list structure is now correct.
            // block = block->prev; // Conceptually, but not needed here.
        } else {
             printf("DEBUG: Previous block %p is FREE but not adjacent.\n", (void*)block->prev);
        }
    }
     printf("DEBUG: Coalescing finished.\n");
}

// Map memory using mmap for large allocations
// Returns pointer to user data area, or NULL on failure
static void *map_memory(size_t size, block_meta_t *last_entry) {
    // Request size + metadata space.
    // MAP_ANONYMOUS: Memory not backed by a file, initialized to zero.
    // MAP_PRIVATE: Modifications are private to this process.
    block_meta_t *block = mmap(NULL, // Let kernel choose address
                             sizeof(block_meta_t) + size,
                             PROT_READ | PROT_WRITE, // Permissions
                             MAP_PRIVATE | MAP_ANONYMOUS,
                             -1, // File descriptor (none for anonymous)
                             0); // Offset (0 for anonymous)

    DIE(block == MAP_FAILED, "mmap failed"); // Check for error

    // Initialize metadata
    block->size = size;
    block->status = STATUS_MAPPED; // Mark as mmap-ed
    block->next = NULL;
    block->prev = last_entry;

    // Link into the list
    if (last_entry) {
        last_entry->next = block;
    } else {
        head = block; // First block overall
    }
    printf("DEBUG: Mapped new block at %p (size %zu) for request %zu\n",
           (void*)block, size, size);

    // Return pointer to user data area
    return (void *)(block + 1);
}

// main functions

void *alt_malloc(size_t size) {
    // Handle invalid size
    if (size == 0) {
        return NULL;
    }

    //Align the requested size
    size = ALIGN8(size);

    // Check for large allocation 
    if (size >= MMAP_THRESHOLD) {
        printf("DEBUG: Request size %zu >= threshold %d. Using mmap.\n", size, MMAP_THRESHOLD);
        block_meta_t *last_entry = find_last_entry(); // Still need last entry to link
        return map_memory(size, last_entry);
    }

    //  If size < MMAP_THRESHOLD, proceed with sbrk logic

    //Search for a suitable free block (Best-Fit)
    block_meta_t *best_block = find_best_fit(size); // find_best_fit ignores MAPPED blocks

    if (best_block) {
        // Block splitting logic remains the same
        printf("DEBUG: Found suitable free block at %p (size %zu) for request %zu\n",
               (void*)best_block, best_block->size, size);

        size_t remaining_size = best_block->size - size;
        if (remaining_size >= sizeof(block_meta_t) + ALIGN8(1)) {
             printf("DEBUG: Splitting block...\n");
             // split logic
        } else {
             printf("DEBUG: Not enough space to split...\n");
        }
        best_block->status = STATUS_ALLOC;
        return (void *)(best_block + 1);

    } else {
        //  No Suitable fre sbrk Block Found 
        printf("DEBUG: No suitable free block found. Requesting new space via sbrk.\n");
        block_meta_t *last_entry = find_last_entry();
        block_meta_t *new_block = request_space(size, last_entry); // request_space uses sbrk
        if (!new_block) return NULL;
        return (void *)(new_block + 1);
    }
}

void alt_free(void *ptr) {
    if (ptr == NULL) {
        return;
    }

    block_meta_t *block = get_block_ptr(ptr);

    if (block->status == STATUS_ALLOC) {
        // --- Freeing sbrk block ---
        block->status = STATUS_FREE;
        printf("DEBUG: Marked sbrk block at %p (user ptr %p, size %zu) as FREE\n",
               (void*)block, ptr, block->size);
        join_free_blocks(block); // Coalesce sbrk blocks

    } else if (block->status == STATUS_MAPPED) {
        //  Freeing mmap block 
         printf("DEBUG: Freeing MAPPED block at %p (user ptr %p, size %zu) using munmap\n",
               (void*)block, ptr, block->size);

        // Unlink the block from the list
        if (block->prev) {
            block->prev->next = block->next;
        } else {
            // Block was the head of the list
            head = block->next;
        }
        if (block->next) {
            block->next->prev = block->prev;
        }

        // Release memory back to the OS
        // We need to unmap the metadata AND the user data area
        size_t total_size = sizeof(block_meta_t) + block->size;
        int ret = munmap(block, total_size);
        DIE(ret == -1, "munmap failed");

    } else if (block->status == STATUS_FREE) {
        printf("DEBUG: Warning - block at %p (user ptr %p) already FREE.\n", (void*)block, ptr);
    }
}

void *alt_calloc(size_t nmemb, size_t size) {
    // Calculate total size and check for overflow / zero request
    size_t total_size = nmemb * size;
    if (nmemb == 0 || size == 0 || total_size / nmemb != size) { // Check for overflow
        return NULL;
    }

    // Align the total size
    total_size = ALIGN8(total_size);

    //  Decide between mmap and sbrk based on threshold
    void *ptr = NULL;
    if (total_size >= MMAP_THRESHOLD) {
        printf("DEBUG calloc: Request size %zu >= threshold. Using mmap.\n", total_size);
        block_meta_t *last_entry = find_last_entry();
        ptr = map_memory(total_size, last_entry);
        // mmap anonymous memory is already zeroed by the kernel
        if (!ptr) return NULL; // map_memory failed

    } else {
        printf("DEBUG calloc: Request size %zu < threshold. Using sbrk logic.\n", total_size);
        // Use the same logic as alt_malloc for finding/allocating sbrk blocks
        block_meta_t *block = find_best_fit(total_size);
        if (block) {
            // Found a free block
            printf("DEBUG calloc: Found suitable free block at %p\n", (void*)block);
             // Split if necessary (same logic as malloc)
            size_t remaining_size = block->size - total_size;
             if (remaining_size >= sizeof(block_meta_t) + ALIGN8(1)) {
                printf("DEBUG calloc: Splitting block.\n");
                block_meta_t *new_free_block = (block_meta_t *)((char *)(block + 1) + total_size);
                new_free_block->size = remaining_size - sizeof(block_meta_t);
                new_free_block->status = STATUS_FREE;
                new_free_block->next = block->next;
                new_free_block->prev = block;
                if (block->next) block->next->prev = new_free_block;
                block->next = new_free_block;
                block->size = total_size;
            } else {
                 printf("DEBUG calloc: Not splitting block.\n");
            }
            block->status = STATUS_ALLOC;
            ptr = (void *)(block + 1);

        } else {
            // No free block, request new space
             printf("DEBUG calloc: No suitable free block. Requesting new space via sbrk.\n");
            block_meta_t *last_entry = find_last_entry();
            block = request_space(total_size, last_entry);
            if (!block) return NULL; // sbrk failed
            ptr = (void *)(block + 1);
        }

        // Zero the memory for sbrk allocations
        if (ptr) {
             printf("DEBUG calloc: Zeroing %zu bytes for sbrk allocation at %p\n", total_size, ptr);
            memset(ptr, 0, total_size);
        }
    }

    return ptr;
}

void *alt_realloc(void *ptr, size_t size) {
  (void)ptr;
  (void)size; // Prevent unused variable warnings
  fprintf(stderr, "alt_realloc not implemented yet!\n");
  return NULL;
}