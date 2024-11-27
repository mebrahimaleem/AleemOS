//AleemOS.c
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
