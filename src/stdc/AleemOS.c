//AleemOS.c

#ifndef __STDC_ALEEMOS_C
#define __STDC_ALEEMOS_C
#include <stdint.h>
#include "stdlib.h"
#include "string.h"
#include "AleemOS.h"

int blink(int x, int y){
	_syscall(2, (uint32_t)(y * 80 + x));
	return 0;
}

int cursorPos(){
	return (int)_syscall(4, 0);
}

void _ls(const char* path) {
	char* params = (char*)malloc(strlen(path)+1);
	strcpy(params, path);
	_syscall(7, (uint32_t)params - __PROCESS_HEAP_BASE);
}

#endif
