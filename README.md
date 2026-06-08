# AltMem: A Custom Memory Allocator in C

AltMem is a custom user-space memory allocator implemented in C. It serves as an educational and practical demonstration of heap memory management, showcasing how standard library functions like `malloc` and `free` operate under the hood using OS-level system calls.

---

## Features

- **8-Byte Alignment**: Ensures all allocated blocks are aligned to 8-byte boundaries for CPU access efficiency.
- **Doubly-Linked List**: Uses a metadata header (`block_meta_t`) prepended to each block to manage free and allocated blocks.
- **System Call Integration**: Dynamically requests heap memory from the operating system using `sbrk` for small allocations and `mmap` for large allocations.
- **Basic Memory Operations**: Exposes functions for allocation and deallocation (`alt_malloc` and `alt_free`).
- **Best-Fit Block Reuse**: Traverses the block list using a Best-Fit strategy to find the smallest free block that satisfies the allocation request, minimizing wasted space.
- **Block Splitting**: Splits a reused free block if it is larger than the requested size plus metadata, reserving the remaining space for future allocations to prevent internal fragmentation.
- **Coalescing (Block Merging)**: Automatically merges adjacent free blocks (both forward and backward) during `alt_free` to combat memory fragmentation.
- **`mmap` Support**: Large allocations (>= 128 KiB) bypass the custom `sbrk` heap and are allocated directly from the OS using anonymous virtual memory mapping (`mmap`). When freed, these blocks are immediately released back to the OS using `munmap`.

---

## Project Structure

- [alt_mem.h](file:///home/subhrojyoti/Code/mem_alloc/alt_mem.h): Public header declaring the allocator's API.
- [alt_mem.c](file:///home/subhrojyoti/Code/mem_alloc/alt_mem.c): Core implementation including internal metadata structures, block tracking, and allocation logic.
- [main.c](file:///home/subhrojyoti/Code/mem_alloc/main.c): Simple test harness to demonstrate allocator operations.

---

## API Reference

### `void *alt_malloc(size_t size)`
Allocates `size` bytes of memory.
- **Returns**: A pointer to the user payload space, or `NULL` if allocation fails.
- **Details**: Aligns the requested size to an 8-byte multiple. If the size is less than 128 KiB, it searches for a best-fit free block, splitting it if possible; otherwise, it extends the heap using `sbrk`. For allocations of 128 KiB or larger, it requests memory from the OS using `mmap`.

### `void alt_free(void *ptr)`
Frees the memory space pointed to by `ptr`.
- **Details**: Retrieves the metadata block preceding the user pointer. If the block was allocated via `sbrk` (marked as `STATUS_ALLOC`), it marks its status as `STATUS_FREE` and coalesces adjacent free blocks. If it was allocated via `mmap` (marked as `STATUS_MAPPED`), it unmaps the memory using `munmap` and updates the block list. If `ptr` is `NULL`, no action is taken.

### `void *alt_calloc(size_t nmemb, size_t size)` *(Planned)*
Allocates zero-initialized memory for an array of `nmemb` elements, each of `size` bytes.

### `void *alt_realloc(void *ptr, size_t size)` *(Planned)*
Resizes the memory block pointed to by `ptr` to `size` bytes.

---

## How to Build and Run

To compile and run the included test suite, follow these steps:

1. **Compile**:
   ```bash
   gcc alt_mem.c main.c -o allocator_test
   ```

2. **Execute**:
   ```bash
   ./allocator_test
   ```

---

## Current Implementation Status & Roadmap

Currently, the allocator supports best-fit block reuse, block splitting, coalescing of adjacent free blocks, and `mmap` support for large allocations. Future enhancements include:

1. **API Completeness**: Fully implement `alt_calloc` and `alt_realloc`.
