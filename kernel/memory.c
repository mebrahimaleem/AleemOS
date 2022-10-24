//memory.c
//
//Implementation for Heap memory management functions

#include <stdint.h>

//The heap will be a linked list of BlockDescriptors. The next BlockDescriptor is 'size' bytes after the previous. Each descritor is 4 bytes large.


typedef struct BlockDescriptor {
	uint32_t size : 30;
	uint32_t flags : 2; //0 = Last descriptor, 1 = Used, 2 = free
} BlockDescriptor;

volatile BlockDescriptor* volatile HeapBase = (BlockDescriptor*)0x100000;

extern inline void initHeap(){
	HeapBase->flags = 0;
	return;
}

extern inline volatile void* volatile malloc(volatile uint32_t size){
	volatile BlockDescriptor* volatile block = HeapBase;
	while (1){
		if (block->flags == 0){
			block->flags = 1;
			block->size = size;
			((volatile BlockDescriptor* volatile)(((volatile void* volatile)block)+4+size))->flags = 0;
			break;
		}
		else if (block->flags == 2){
			if (block->size >= size+5){
				block->flags = 1;
				((volatile BlockDescriptor* volatile)(((volatile void* volatile)block)+4+size))->flags = 2;
				((volatile BlockDescriptor* volatile)(((volatile void* volatile)block)+4+size))->size = block->size - size - 4;
				break;
			}
			else if (block->size >= size){
				block->flags = 1;
				break;
			}
		}
		block = (volatile BlockDescriptor* volatile)(((volatile void* volatile)block)+4 + block->size);
		continue;
	}

	return ((volatile void* volatile)block)+4;
}

extern inline void free(volatile void* volatile block){
	volatile BlockDescriptor* volatile descriptor = (volatile BlockDescriptor* volatile)(block - 4);	
	if (((volatile BlockDescriptor* volatile)(block + descriptor->size))->flags == 0)
		descriptor->flags = 0;

	else if (((volatile BlockDescriptor* volatile)(block + descriptor->size))->flags == 2){
		descriptor->flags = 2;
		descriptor->size += 4 + ((volatile BlockDescriptor* volatile)(block + descriptor->size))->size;
	}

	else
		descriptor->flags = 2;

	return;
}
