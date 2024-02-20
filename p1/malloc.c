#include "malloc.h"

#include <stdio.h>
#include <assert.h>

/*
 * This is the heap you should use.
 * The heap does not grow.
 */
uint8_t __attribute__ ((aligned(HEADER_SIZE))) _heapData[HEAP_SIZE];

/*
 * This should point to the first free block in memory.
 */
Block *_firstFreeBlock;

/*
 * Initializes the memory block. You don't need to change this.
 */
void initAllocator()
{
	_firstFreeBlock = (Block*)&_heapData[0];
	_firstFreeBlock->next = NULL;
	_firstFreeBlock->size = HEAP_SIZE;
}

/*
 * Gets the next block that should start after the current one.
 */
static Block *_getNextBlockBySize(const Block *current)
{
	static const Block *end = (Block*)&_heapData[HEAP_SIZE];
	Block *next = (Block*)&current->data[current->size - HEADER_SIZE];

	assert(next <= end);
	return (next == end) ? NULL : next;
}

/*
 * Dumps the allocator. You should not need to modify this.
 */
void dumpAllocator()
{
	Block *current;
	/* Note: This sample code prints addresses relative to the beginning of the heap */

	/* Part a: all blocks, using size, and starting at the beginning of the heap */
	printf("All blocks:\n");
	current = (Block*)&_heapData[0];
	while (current) {
		printf("  Block starting at %" PRIuPTR ", size %" PRIu64 " (%s)\n",
			((uintptr_t)(void*)current - (uintptr_t)(void*)&_heapData[0]),
			current->size,
			current->magic == ALLOCATED_BLOCK_MAGIC ? "in use" : "free" );

		current = _getNextBlockBySize(current);
	}

	/* Part b: free blocks, using next pointer, and starting at _firstFreeBlock */
	printf("Free block list:\n");
	current = _firstFreeBlock;
	while (current) {
		printf("  Free block starting at %" PRIuPTR ", size %" PRIu64 "\n",
			((uintptr_t)(void*)current - (uintptr_t)(void*)&_heapData[0]),
			current->size);

		current = current->next;
	}
}

/*
 * Round the integer up to the block header size (16 Bytes).
 */
uint64_t roundUp(uint64_t n)
{
	unsigned int mask = 0x0F;
	int ret_val = (n + mask) & ~mask;
	return ret_val;
}

/*
 * Internal helper function for making a new allocation
 * (not needed for any tests, but a suggested structure to simplify your life)
 * This function should basically do the last two items of the assignment description
 * of part b (my_malloc).
 * - block is the (currently free) block that you want to use for your new allocation
 * - new_size is the total size for the new allocation (size requested in the
 *   my_malloc call plus header size)
 * - update_next is a pointer to the next pointer pointing to block. This is
 *   the pointer you need to update for removing block from the free-list/
 *   replace block with a new free block starting somewhere within block
 */
static void * __attribute__ ((unused)) allocate_block(Block **update_next, Block *block, uint64_t new_size) {

	// if (block->size > new_size + HEADER_SIZE){
	// 	uint64_t remaining_size = block->size - new_size;
	// 	Block* new_free_block = (Block*)(block->data + new_size);
	// 	new_free_block->size = remaining_size;
	// 	*update_next = new_free_block;
	// 	block->size = new_size;
	// 	block->magic = ALLOCATED_BLOCK_MAGIC;
	// }else{
	// 	*update_next = block->next;
	// }

	// return NULL;
	// Ensure new_size includes the block header

    new_size = roundUp(new_size + sizeof(Block));

    if (block->size >= new_size + sizeof(Block)) { 
        uint64_t remaining_size = block->size - new_size;
        Block* new_free_block = (Block*)((uint8_t*)block + new_size);
        new_free_block->size = remaining_size;
        new_free_block->next = block->next; 
        *update_next = new_free_block; 
        block->size = new_size;
    } else {
        *update_next = block->next; 
    }
    block->magic = ALLOCATED_BLOCK_MAGIC; 

    return block->data; 
}


void *my_malloc(uint64_t size)
{
	size = roundUp(size);

	Block *free_block = _firstFreeBlock;
	Block **update_next = &_firstFreeBlock;

	printf("size : %llu, free space in heap : %llu \n", size, free_block->size);

	//while(free_block == NULL || free_block->size < size){
	while(free_block != NULL && free_block->size < size + HEADER_SIZE){
		if(free_block->next == NULL){
			printf("no space in heap, memory not allocated\n");
			return NULL;
		}
		free_block = free_block->next;
	}

	allocate_block(update_next, free_block, size);
	

	// TODO: Implement
	// Suggested approach: Search for a free block, then call allocate_block with that block
 	// (and suitable values for update_next and new_size)
	// This is not mandatory, what counts in the and is that my_malloc does the right thing.
	return NULL;
}

/*
 * Internal helper function to merge two blocks.
 * (not needed for any tests, but a suggested structure to simplify your life)
 * Suggested functionality (to be used in my_free):
 * if block2 follows directly after block1, then merge these two (free) blocks
 * i.e. update block1 such that it becomes one larger free block
 */
static void __attribute__ ((unused)) merge_blocks(Block *block1, Block *block2)
{
	if ((uint8_t*)block1 + block1->size == (uint8_t*)block2) {
        block1->size += block2->size;
        block1->next = block2->next;
	}

}


void my_free(void *address)
{
	// Block *block = (Block *)((uint8_t*)address - HEADER_SIZE);
	// Block *current = _firstFreeBlock;
    // while (current != NULL) {
    //     if ((uint8_t*)current + current->size == (uint8_t*)block || (uint8_t*)block + block->size == (uint8_t*)current) {
    //         merge_blocks(current, block);
    //         return; 
    //     }
    //     current = current->next;
    // }


	if (!address) return;

    Block *blockToFree = (Block *)((uint8_t*)address - HEADER_SIZE);
    Block *prev = NULL;
    Block *current = _firstFreeBlock;

    while (current != NULL && (uint8_t*)current < (uint8_t*)blockToFree) {
        if ((uint8_t*)current + current->size == (uint8_t*)blockToFree) {
            merge_blocks(current, blockToFree);
            blockToFree = current; 
        }
        prev = current;
        current = current->next;
    }

    if (prev != NULL && (uint8_t*)blockToFree + blockToFree->size == (uint8_t*)current) {
        merge_blocks(blockToFree, current);
    } else {
        blockToFree->next = current;
        if (prev != NULL) {
            prev->next = blockToFree;
        } else {
            _firstFreeBlock = blockToFree;
        }
    }

    if (current != NULL && (uint8_t*)blockToFree + blockToFree->size == (uint8_t*)current) {
        merge_blocks(blockToFree, current);
    }





}


