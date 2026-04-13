# Embedded C Memory Management Suite

A custom memory management library written in C. This project includes two different memory allocators designed for Embedded Systems and Real-Time Operating Systems (RTOS).

## Key Features

* Memory Alignment: Automatically aligns the base memory address and all blocks to 16 bytes. This prevents hardware errors and improves CPU performance.
* Static Memory: Uses pre-allocated static arrays instead of the OS heap. This makes it deterministic and suitable for bare-metal systems.
* Encapsulation: Uses the 'static' keyword to hide internal variables and prevent naming conflicts with other files.
* Safe Design: Built with best practices to help developers avoid dangling pointers and memory leaks.

## The Allocators

### 1. Dynamic Allocator (dyn_alloc)
An allocator for variable memory sizes. It uses an Explicit Free List with headers and footers.
* Splitting: Cuts large blocks into smaller ones to save memory.
* Coalescing: Automatically merges free blocks together to prevent memory fragmentation.
* Use Case: Good for variable-size data like strings, dynamic buffers, and packets.

### 2. Fixed-Size Memory Pool (pool_alloc)
A fast allocator that divides memory into fixed blocks of 64 bytes.
* Performance: O(1) constant time for allocation and deallocation.
* No Fragmentation: Because all blocks are the same size, external fragmentation is impossible.
* Use Case: Good for fast, repetitive tasks like RTOS task control blocks (TCBs) or fixed sensor data.

## Design Overview

The library separates two allocation strategies:

- `dyn_alloc` → flexible memory management with fragmentation handling
- `pool_alloc` → deterministic, real-time safe allocation

Both allocators operate on statically allocated memory buffers.

## Usage Example

```c
#include <stdio.h>
#include "dyn_alloc.h"
#include "memory_pool.h"

int main() {
    // --- Dynamic Allocator ---
    dyn_alloc_init();
    void* buffer = dyn_alloc(128);
    dyn_free(buffer);
    buffer = NULL; // Prevent dangling pointers

    // --- Fixed-Size Pool ---
    pool_init();
    void* packet = pool_alloc();
    pool_free(packet);
    packet = NULL;

    return 0;
}
```