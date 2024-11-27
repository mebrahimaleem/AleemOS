//memory.c
//
//Implementation for Heap memory management functions

#include <stdint.h>
#include <memory.h>

//The heap will be a linked list of BlockDescriptors. The next BlockDescriptor is 'size' bytes after the previous. Each descritor is 4 bytes large.

BlockDescriptor* HeapBase = (BlockDescriptor*)0x100000;

//Sets up the kernel heap, root BlockDescriptor is @ 0x100000
void initHeap(void){
	HeapBase->flags = 0;
	return;
}

inline void* malloc(uint32_t size) {
	return _malloc(size, HeapBase);
}

//Allocates a new block of memory of size 'size'
void* _malloc(uint32_t size, BlockDescriptor* block){
	//New block descriptor to use
	while (1){
		if (block->flags == 0){ //Check if last BlockDescriptor (remainder of Heap is free)
			block->flags = 1;
			block->size = size & 0x3FFFFFFF;
			((BlockDescriptor* )(((uint8_t* )block)+4+size))->flags = 0;
			break;
		}
		else if (block->flags == 2){ //Check if block is free, but there is another block ahead
			if (block->size >= size+5){ //There is enough room to allocate the block and another BlockDescriptor
				block->flags = 1;
				((BlockDescriptor* )(((uint8_t* )block)+4+size))->flags = 2;
				((BlockDescriptor* )(((uint8_t* )block)+4+size))->size = (block->size - size - 4) & 0x3FFFFFFF;
				break;
			} //Otherwise use the entire block
			else if (block->size >= size){
				block->flags = 1;
				break;
			}
		}
		//Calculate next block descriptor
		block = (BlockDescriptor* )(((uint8_t* )block)+4 + block->size);
		continue;
	}

	return (void*)(((uint8_t* )block)+4);
}

//Free memory given the start of an allocated block
inline void free(void* block){
	//BlockDescriptor is 4 bytes behind allocated block
	BlockDescriptor* descriptor = (BlockDescriptor* )((uint8_t* )block - 4);

	if (((BlockDescriptor* )((uint8_t* )block + descriptor->size))->flags == 0) //Check if freed block is to become last block
		descriptor->flags = 0;

	else if (((BlockDescriptor* )((uint8_t* )block + descriptor->size))->flags == 2){ //See if we can merge two blocks
		descriptor->flags = 2;
		descriptor->size = 0x3FFFFFFF &  
			(uint32_t)(descriptor->size + 4 + ((BlockDescriptor* )((uint8_t* )block + descriptor->size))->size);
	}

	else //Otherwise set block to unused
		descriptor->flags = 2;

	return;
}
