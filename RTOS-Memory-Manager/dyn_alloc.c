#include "dyn_alloc.h"
#include <stdio.h>

#define BUFFER_SIZE 10000
#define ALIGNMENT 16 
#define ALIGN(size)      (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT -1))
#define HEADER_SIZE       ALIGN(sizeof(BlockHeader))

typedef struct BlockHeader {
	struct BlockHeader* prev;
	struct BlockHeader* next;
	size_t size;
}BlockHeader;

// Static keeps this private for this file only
static uint8_t array[BUFFER_SIZE];
static BlockHeader* head = NULL;


void dyn_alloc_init() {

	// Align the first address in the array
	uintptr_t base_addr = (uintptr_t)array;
	uintptr_t aligned_addr = ALIGN(base_addr);

	size_t lost_space = aligned_addr - base_addr;
	size_t usable_space = BUFFER_SIZE - lost_space;
	head = (BlockHeader*)aligned_addr;

	// Set initial block size and mark "previous" block as allocated to prevent illegal merging
	head->size = (usable_space - HEADER_SIZE) | 2;
	head->next = NULL;
	head->prev = NULL;
}


void* dyn_alloc(size_t size) {
	if (size == 0) {
		return NULL;
	}

	size = ALIGN(size);
	BlockHeader* curr = head;
	uint8_t* end_ptr = array + BUFFER_SIZE;

	while (curr) {
		// Clean the flags to get the actual block size
		size_t actual_size = curr->size & ~0xF;
		size_t prev_status = curr->size & 2;

		if (size <= actual_size) { // Check if there is enough space in this block

			// Check if we can split this block into two
			if (size + HEADER_SIZE + ALIGNMENT <= actual_size) {
				// Create a new free block after the allocated space
				BlockHeader* new_block = (BlockHeader*)((uint8_t*)curr + HEADER_SIZE + size);
				size_t remaining_size = actual_size - size - HEADER_SIZE;

				// Set new block size and mark that the block before it (curr) is now allocated
				new_block->size = remaining_size | 2;
				new_block->next = curr->next;
				new_block->prev = curr->prev;

				// Update the free list pointers
				if (curr->prev) {
					curr->prev->next = new_block;
				}
				else {
					head = new_block;
				}
				if (curr->next) {
					curr->next->prev = new_block;
				}

				// Write the footer for the new free block
				size_t* new_footer = (size_t*)((uint8_t*)new_block + HEADER_SIZE + remaining_size - sizeof(size_t));
				*new_footer = new_block->size;

				// Set the allocated block's size and flags and remember prev status
				curr->size = size | 1 | prev_status;
				return (void*)((uint8_t*)curr + HEADER_SIZE);
			}
			else { // Not enough space for a new header, use the whole block
				// Remove the block from the free list
				if (curr->prev) {
					curr->prev->next = curr->next;
				}
				else {
					head = curr->next;
				}
				if (curr->next) {
					curr->next->prev = curr->prev;
				}

				// Mark as allocated and remember prev status
				curr->size = actual_size | 1 | prev_status;

				// Notify the next physical block that its neighbor (curr) is now allocated
				BlockHeader* next_phys = (BlockHeader*)((uint8_t*)curr + HEADER_SIZE + actual_size);
				if ((uint8_t*)next_phys < end_ptr) {
					next_phys->size |= 2;

				}
				return (void*)((uint8_t*)curr + HEADER_SIZE);
			}
		}
		curr = curr->next; // Move to the next free block in the list
	}
	return NULL;
}

void dyn_free(void* ptr) {
	if (ptr == NULL) {
		return;
	}

	// Go back to the header of the block
	BlockHeader* curr = (BlockHeader*)((uint8_t*)ptr - HEADER_SIZE);
	curr->size = curr->size & ~1; // Set flag to 0 (Free)

	uint8_t* end_ptr = array + BUFFER_SIZE;
	// Find the next physical block in memory
	BlockHeader* next_phys = (BlockHeader*)((uint8_t*)curr + HEADER_SIZE + (curr->size & ~0xF));

	//  Check if the next block is free and merge it
	if ((uint8_t*)next_phys < end_ptr && (next_phys->size & 1) == 0) {
		// Remove the next block from the free list
		if (next_phys->prev) {
			next_phys->prev->next = next_phys->next;
		}
		else {
			head = next_phys->next;
		}
		if (next_phys->next) {
			next_phys->next->prev = next_phys->prev;
		}

		// Add the next block's size and its header to the current block
		curr->size += (next_phys->size & ~0xF) + HEADER_SIZE;
	}

	// Check if the previous block is free
	if ((curr->size & 2) == 0) {
		// Read the footer of the previous block
		size_t* prev_footer = ((size_t*)curr) - 1;
		size_t prev_size = (*prev_footer) & ~0xF;
		// Find the start of the previous block header
		BlockHeader* prev_block = (BlockHeader*)((uint8_t*)curr - prev_size - HEADER_SIZE);

		// Merge current block into the previous one
		prev_block->size += (curr->size & ~0xF) + HEADER_SIZE;
		curr = prev_block; // 'curr' now points to the combined block
	}
	else { // Previous block is allocated, so just add current block to the free list
		curr->next = head;
		curr->prev = NULL;
		if (head) {
			head->prev = curr;
		}
		head = curr;
	}

	// Update the footer for the newly freed/merged block
	size_t* my_footer = (size_t*)((uint8_t*)curr + HEADER_SIZE + (curr->size & ~0xF) - sizeof(size_t));
	*my_footer = curr->size;

	// Notify the next physical block that we are now free
	BlockHeader* new_next_phys = (BlockHeader*)((uint8_t*)curr + HEADER_SIZE + (curr->size & ~0xF));
	if ((uint8_t*)new_next_phys < end_ptr) {
		new_next_phys->size &= ~2; // Mark that neighbor before is now free
	}
}