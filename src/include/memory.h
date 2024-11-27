//memory.h
//
//This file has declarations for all Heap operations
/*
MIT License

Copyright 2022-2024 Ebrahim Aleem

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
*/

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
