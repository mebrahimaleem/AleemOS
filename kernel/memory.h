//memory.h
//
//This file has declarations for all Heap operations

extern void initHeap();

extern volatile void* volatile malloc(volatile uint32_t size);

extern void free(volatile void* volatile block);
