#include "memory_pool.h"
#include <stdio.h>

#define BLOCK_SIZE 64                  
#define ALIGNMENT 16                   
#define POOL_SIZE (1024 + ALIGNMENT - 1) 
#define ALIGN(size)      (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT -1))

typedef struct Header {
	struct Header* next;
} Header;

static Header* pool_head = NULL;
static uint8_t array[POOL_SIZE];

void pool_init() {
	uintptr_t base_addr = (uintptr_t)array;
	uintptr_t aligned_addr = ALIGN(base_addr);       // Find the first 16-byte aligned address
	size_t lost_space = aligned_addr - base_addr;    // How many bytes we skipped for alignment

	pool_head = (Header*)aligned_addr;               // The first block starts here

	// Calculate how many blocks fit in the remaining space
	size_t number_of_blocks = (POOL_SIZE - lost_space) / BLOCK_SIZE;
	Header* curr = pool_head;

	// Loop to connect the blocks together into a linked list
	for (int i = 0; i < number_of_blocks - 1; i++) {
		curr->next = (Header*)((uint8_t*)curr + BLOCK_SIZE);
		curr = curr->next;
	}
	// The end of the linked list
	curr->next = NULL;
}

void* pool_alloc() {
	if (pool_head == NULL) {
		return NULL;
	}

	Header* temp = pool_head;          // Take the current head
	pool_head = pool_head->next;       // Move head to the next available block
	return (void*)temp;                // Return the block to the user
}

void pool_free(void* ptr) {
	if (ptr == NULL) {
		return;
	}

	Header* new_header = (Header*)ptr;
	new_header->next = pool_head;     // Connect the pointer to the head
	pool_head = new_header;           // Put the pointer to be the head
}