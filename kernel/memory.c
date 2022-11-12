//memory.c
//
//Implementation for Heap memory management functions

#include <stdint.h>
#include "memory.h"

//The heap will be a linked list of BlockDescriptors. The next BlockDescriptor is 'size' bytes after the previous. Each descritor is 4 bytes large.


//Block Descriptor Datastructure for Heap Allocation
typedef struct BlockDescriptor {
	uint32_t size : 30; //Size of block
	uint32_t flags : 2; //0 = Last descriptor, 1 = Used, 2 = free
} __attribute((packed)) BlockDescriptor;

volatile BlockDescriptor* volatile HeapBase = (BlockDescriptor*)0x100000;

//Sets up the kernel heap, root BlockDescriptor is @ 0x100000
inline void initHeap(void){
	HeapBase->flags = 0;
	return;
}

//Allocates a new block of memory of size 'size'
inline void* malloc(volatile uint32_t size){
	//New block descriptor to use
	volatile BlockDescriptor* volatile block = HeapBase;
	while (1){
		if (block->flags == 0){ //Check if last BlockDescriptor (remainder of Heap is free)
			block->flags = 1;
			block->size = size;
			((volatile BlockDescriptor* volatile)(((volatile void* volatile)block)+4+size))->flags = 0;
			break;
		}
		else if (block->flags == 2){ //Check if block is free, but there is another block ahead
			if (block->size >= size+5){ //There is enough room to allocate the block and another BlockDescriptor
				block->flags = 1;
				((volatile BlockDescriptor* volatile)(((volatile void* volatile)block)+4+size))->flags = 2;
				((volatile BlockDescriptor* volatile)(((volatile void* volatile)block)+4+size))->size = block->size - size - 4;
				break;
			} //Otherwise use the entire block
			else if (block->size >= size){
				block->flags = 1;
				break;
			}
		}
		//Calculate next block descriptor
		block = (volatile BlockDescriptor* volatile)(((volatile void* volatile)block)+4 + block->size);
		continue;
	}

	return ((volatile void* volatile)block)+4;
}

//Free memory given the start of an allocated block
inline void free(volatile void* volatile block){
	volatile BlockDescriptor* volatile descriptor = (volatile BlockDescriptor* volatile)(block - 4); //BlockDescriptor is 4 bytes behind allocated block
	if (((volatile BlockDescriptor* volatile)(block + descriptor->size))->flags == 0) //Check if freed block is to become last block
		descriptor->flags = 0;

	else if (((volatile BlockDescriptor* volatile)(block + descriptor->size))->flags == 2){ //See if we can merge two blocks
		descriptor->flags = 2;
		descriptor->size += 4 + ((volatile BlockDescriptor* volatile)(block + descriptor->size))->size;
	}

	else //Otherwise set block to unused
		descriptor->flags = 2;

	return;
}
