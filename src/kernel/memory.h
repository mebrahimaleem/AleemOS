//memory.h
//
//This file has declarations for all Heap operations

/*
	Setups up the starting node in the headp
*/
extern void initHeap(void);

/*
	Allocates memory
	size: Number of bytes to allocate

	Returns: Pointer to allocated memory
*/
extern void* malloc(volatile uint32_t size);

/*
	Frees allocated memory
	block: Pointer to allocated memory
*/
extern void free(volatile void* volatile block);
