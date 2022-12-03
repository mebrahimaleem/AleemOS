//stdio.c

#ifndef __STDC_STDIO_C
#define __STDC_STDIO_C
#include <stdint.h>
#include <stdarg.h>
#include "stdio.h"

int printf(const char* format, ...){
	va_list args;
	va_start(args, format);

	uint32_t ret = 0;

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

int scanf(const char* format, ...){
	va_list args;
	va_start(args, format);

	uint32_t ret;
	for (const char* i = format; *i != 0; i++){
		uint32_t buf;
		if (*i == '%'){
			i++;
			switch (*i){
				case 'c':
					ret++;
					asm volatile ("pushad \n push 3 \n push 0 \n mov eax, esp \n int 0x30 \n add esp, 8 \n mov [esp-4], eax \n popad \n mov %0, dword [esp-40]"
					 	: "=b"(buf) : : "memory"); //NOTE: esp-40 Should actually be esp-36, but gcc subtracts 4 (still compiles to esp-36, esp-36 would compile to esp-32)
					*(char*)(va_arg(args, char*)) = (char)buf;
					break;
				default: //Bad specifier
					return -1;
			}
		}
		else {
			asm volatile ("pushad \n push 3 \n push 0 \n mov eax, esp \n int 0x30 \n add esp, 8 \n mov %0, eax \n popad" : "=m"(buf) : : "memory");
			if ((uint8_t)buf != (uint8_t)*i) return -1;
			continue;
		}
	}

	return (int)ret;
}
#endif
