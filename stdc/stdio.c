//stdio.c

#ifndef __STDC_STDIO_C
#define __STDC_STDIO_C
#include <stdint.h>
#include <stdarg.h>
#include "stdio.h"

int blink(int x, int y){
	asm volatile ("pushad \n push 2 \n push %0 \n mov eax, esp \n int 0x30 \n add esp, 8 \n popad" : : "b"((uint32_t)(y * 80 + x)) : "memory");
	return 0;
}

int printf(const char* format, ...){
	va_list args;
	va_start(args, format);

	uint32_t ret = 0;
	int snum;
	unsigned int num;
	uint8_t* nums;
	char* s;

	for (const char* i = format; *i != 0; i++){
		if (*i == '%'){
			i++;
			switch (*i){ //TODO: Complete formating specifier handling
				case 'c':
					ret++;
					//NOTE: char is promoted to int when using ...
					asm volatile ("pushad \n push 1 \n push %0 \n mov eax, esp \n int 0x30 \n add esp, 8 \n popad" : : "b"((uint32_t)va_arg(args, int)) : "memory");
					break;
				default: //Bad specifier
					return -1;
			}
		}

		else{
			ret++;
			asm volatile ("pushad \n push 1 \n push %0 \n mov eax, esp \n int 0x30 \n add esp, 8 \n popad" : : "b"((uint32_t)*i) : "memory");
		}
	}

	return (int)ret;
}

#endif
