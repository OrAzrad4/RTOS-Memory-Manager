#include <stdio.h>
#include "dyn_alloc.h"
#include "memory_pool.h"

int main() {
    printf("RTOS Memory Allocators Test\n");

    // Test Dynamic Allocator
    dyn_alloc_init();
    void* d_ptr1 = dyn_alloc(128);
    void* d_ptr2 = dyn_alloc(256);
    printf("Dynamic Allocator: Allocated 128 bytes at %p\n", d_ptr1);
    printf("Dynamic Allocator: Allocated 256 bytes at %p\n", d_ptr2);
    dyn_free(d_ptr1); d_ptr1 = NULL;
    dyn_free(d_ptr2); d_ptr2 = NULL;

    // Test Memory Pool
    pool_init();
    void* p_ptr1 = pool_alloc();
    void* p_ptr2 = pool_alloc();
    printf("Memory Pool: Allocated 64-byte block at %p\n", p_ptr1);
    printf("Memory Pool: Allocated 64-byte block at %p\n", p_ptr2);
    pool_free(p_ptr1); p_ptr1 = NULL;
    pool_free(p_ptr2); p_ptr2 = NULL;

    printf("All memory freed successfully!\n");
    return 0;
}