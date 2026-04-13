#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <stdint.h>
#include <stddef.h>

void pool_init(void);
void* pool_alloc(void);
void pool_free(void* ptr);

#endif