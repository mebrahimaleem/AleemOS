//stdlib.c
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

#ifndef STDC_STDLIB_C
#define STDC_STDLIB_C
#include "stdlib.h"
uint32_t __PROCESS_HEAP_BASE;

typedef struct BlockDescriptor {
	uint32_t size : 30;
	uint32_t flags : 2;
} __attribute((packed)) BlockDescriptor;

void* malloc(size_t size){
	BlockDescriptor* block = (BlockDescriptor*)__PROCESS_HEAP_BASE;
	while (1){
		if (block->flags == 0){
			block->flags = 1;
			block->size = size & 0x3FFFFFFF;
			((BlockDescriptor* )(((uint8_t* )block)+4+size))->flags = 0;
			break;
		}
		else if (block->flags == 2){
			if (block->size >= size+5){
				block->flags = 1;
				((BlockDescriptor* )(((uint8_t* )block)+4+size))->flags = 2;
				((BlockDescriptor* )(((uint8_t* )block)+4+size))->size = (block->size - size - 4) & 0x3FFFFFFF;
				break;
			}
			else if (block->size >= size){
				block->flags = 1;
				break;
			}
		}
		block = (BlockDescriptor* )(((uint8_t* )block)+4 + block->size);
		continue;
	}

	return (void*)(((uint8_t* )block)+4);
}

void free(void* ptr){
	BlockDescriptor* desc = (BlockDescriptor*)((uint8_t*)ptr - 4);
	if (((BlockDescriptor* )((uint8_t* )ptr + desc->size))->flags == 0)
		desc->flags = 0;
	
	else if (((BlockDescriptor* )((uint8_t* )ptr + desc->size))->flags == 2){
		desc->flags = 2;
		desc->size = 0x3FFFFFFF &
			(uint32_t)(desc->size + 4 + ((BlockDescriptor* )((uint8_t* )ptr + desc->size))->size);
	}

	else
		desc->flags = 2;

	return;
}

char* itoa(int value, int base){
	char* str = 0;
	char* st = malloc(64);

	if (value == 0){
		str = (char*)malloc(2);
		str[0] = '0';
		str[1] = 0;
		return str;
	}

	if (base == 1){
		str = (char*)malloc((size_t)(value+1));
		for (int i = 0; i < value; i++) str[i] = '1';
		str[value] = 0;
		return str;
	}

	if (base == 0 || base > 37) return (char*)0;

	uint8_t neg = 0;
	if (base == 10 && value < 0){
		neg = 1;
		value = -value;
	}

	if (base == 36) base = 10; //Base 36 is unsigned base 10

	for (uint8_t i = 0; i < 64; i++){
		st[63-i] = (char)(value % base);
		value /= base;
	}

	char* s = 0;
	for (char* i = st; i < st + 64; i++){
		if (*i != 0 && s == 0){
			str = (char*)malloc((uint8_t)(65 - (i - st) + (neg == 1 ? 1 : 0)));
			s = str;
			if (neg == 1){
				s[0] = '-';
				s++;
			}
			s[64 - (i - st)] = 0;
		}
		else if (s == 0) continue;

		if (*i < 10) *s = '0' + *i;
		else *s = 'A' - 10 + *i;
		s++;
	}
	
	free(st);
	return str;
}

void* realloc(void* ptr, size_t size){
	if ((uint32_t)ptr == 0) return malloc(size);
	if (size == 0){
		free(ptr);
		return (void*)0;
	}

	BlockDescriptor* desc = (BlockDescriptor*)((uint8_t*)ptr - 4);
	if (desc->size >= size) return ptr; //Check if we already allocated enough memory	
	if (((BlockDescriptor* )((uint8_t* )ptr + desc->size))->flags == 0){
		desc->size = 0x3FFFFFFF & size;
		((BlockDescriptor* )((uint8_t* )ptr + size))->flags = 0;
		return ptr;
	}

	uint8_t* nb = malloc(size);
	uint8_t* nc = nb;
	for (uint8_t* i = (uint8_t*)ptr; i < (uint8_t*)ptr + desc->size; i++){
		*nc = *i;
		nc++;
	}

	free(ptr);
	return (void*)nb;
}
#endif
