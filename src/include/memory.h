//memory.h
//
//This file has declarations for all Heap operations

//Block Descriptor Datastructure for Heap Allocation
typedef struct BlockDescriptor {
	uint32_t size : 30; //Size of block
	uint32_t flags : 2; //0 = Last descriptor, 1 = Used, 2 = free
} __attribute((packed)) BlockDescriptor;

/*
	Setups up the starting node in the headp
*/
extern void initHeap(void);

/*
	Allocates memory
	size: Number of bytes to allocate

	Returns: Pointer to allocated memory
*/
extern void* malloc(uint32_t size);

extern void* _malloc(uint32_t size, BlockDescriptor* block);

/*
	Frees allocated memory
	block: Pointer to allocated memory
*/
extern void free(void* block);
