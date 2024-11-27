//stdlib.h

#ifndef __STDC_STDLIB_H
#define __STDC_STDLIB_H
#include <stdint.h>
typedef unsigned int size_t;

extern uint32_t __PROCESS_HEAP_BASE;

void* malloc(size_t size);
void free(void* ptr);
void* realloc(void* ptr, size_t size);
char* itoa(int value, int base);
#endif
