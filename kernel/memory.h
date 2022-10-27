//memory.h
//
//This file has declarations for all Heap operations

extern void initHeap(void);

extern void* malloc(volatile uint32_t size);

extern void free(volatile void* volatile block);
