#ifndef DYN_ALLOC_H
#define DYN_ALLOC_H

#include <stdint.h>
#include <stddef.h>

void dyn_alloc_init(void);
void* dyn_alloc(size_t size);
void dyn_free(void* ptr);

#endif