#pragma once 
#include <stddef.h> // Provides size_t type

/*
 * Allocates 'size' bytes of uninitialized memory.
 * Returns: A pointer to the allocated memory, or NULL if the request fails.
 */
void *alt_malloc(size_t size);

/*
 * Frees the memory space pointed to by 'ptr', which must have been returned
 * by a previous call to alt_malloc(), alt_calloc() or alt_realloc().
 * If 'ptr' is NULL, no operation is performed.
 */
void alt_free(void *ptr);

/*
 * Allocates memory for an array of 'nmemb' elements of 'size' bytes each
 * and returns a pointer to the allocated memory. The memory is set to zero.
 * Returns: A pointer to the allocated memory, or NULL if the request fails.
 */
void *alt_calloc(size_t nmemb, size_t size);

/*
 * Changes the size of the memory block pointed to by 'ptr' to 'size' bytes.
 * The contents will be unchanged in the range from the start of the region
 * up to the minimum of the old and new sizes. If the new size is larger,
 * the added memory will not be initialized.
 * If 'ptr' is NULL, the call is equivalent to alt_malloc(size).
 * If 'size' is 0, the call is equivalent to alt_free(ptr).
 * Returns: A pointer to the newly allocated memory, or NULL if the request fails.
 */
void *alt_realloc(void *ptr, size_t size);