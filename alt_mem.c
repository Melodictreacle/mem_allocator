#include "alt_mem.h" 

#include <stdio.h>      // For perror
#include <stdlib.h>     // For exit, EXIT_FAILURE
#include <unistd.h>     // For sbrk, sysconf, getpagesize
#include <sys/mman.h>   // For mmap, munmap
#include <string.h>     // For memset, memcpy
#include <stdbool.h> // For bool type 


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
#define DIE(assertion, call_description)                                \
    do {                                                                \
        if (assertion) {                                                \
            perror(call_description);                                   \
            exit(EXIT_FAILURE);                                         \
        }                                                               \
    } while (0)

//Data Structures

// Possible states for a memory block
typedef enum {
    STATUS_FREE,    // Block is free and available for allocation
    STATUS_ALLOC,   // Block is currently allocated (used by sbrk)
    STATUS_MAPPED   // Block was allocated using mmap
} block_status;

// Metadata structure prepended to each memory block
typedef struct block_meta {
    size_t size;             // Size of the *user* data area (payload), doesn't include sizeof(struct block_meta)
    block_status status;     // Current status of the block (FREE, ALLOC, MAPPED)
    struct block_meta *next; // Pointer to the next block_meta in the list
    struct block_meta *prev; // Pointer to the previous block_meta in the list
} block_meta_t;

//Global Variables

// Head of the linked list of memory blocks. Initially NULL.
static block_meta_t *head = NULL;

// Flag to track if we've done the initial large allocation
static bool preallocated = false;

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
    block->next = NULL;          // This is the new last block
    block->prev = last_entry;    // Link to the previous block

    // Update the previous block's 'next' pointer if it exists
    if (last_entry) {
        last_entry->next = block;
    } else {
        // If there was no previous block, this is the new head of the list
        head = block;
    }

    return block;
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


void *alt_malloc(size_t size) {
    // 1. Handle invalid size
    if (size == 0) {
        return NULL;
    }

    // 2. Align the requested size to 8 bytes
    size = ALIGN8(size);

    // 3. Find the last block in our list to link the new one after it
    block_meta_t *last_entry = find_last_entry();

    // 4. Request new space using sbrk
    //    (For now, we always request new space)
    block_meta_t *new_block = request_space(size, last_entry);
    if (!new_block) {
        return NULL; // sbrk failed
    }

    // 5. Return the pointer to the user data area
    //    This is right *after* our metadata block
    return (void *)(new_block + 1); // Pointer arithmetic: moves pointer by sizeof(block_meta_t)
}

void alt_free(void *ptr) {
    (void)ptr; // Prevent unused variable warning
    fprintf(stderr, "alt_free not implemented yet!\n");
}

void *alt_calloc(size_t nmemb, size_t size) {
    (void)nmemb; (void)size; // Prevent unused variable warnings
    fprintf(stderr, "alt_calloc not implemented yet!\n");
    return NULL;
}

void *alt_realloc(void *ptr, size_t size) {
    (void)ptr; (void)size; // Prevent unused variable warnings
    fprintf(stderr, "alt_realloc not implemented yet!\n");
    return NULL;
}