//stdio.c

#ifndef __STDC_STDIO_C
#define __STDC_STDIO_C
#include <stdint.h>
#include <stdarg.h>
#include "stdio.h"
#include "stdlib.h"
#include "AleemOS.h"

#pragma GCC push_options
#pragma GCC optimize("O0")
int printf(const char* format, ...){
	va_list args;
	va_start(args, format);

	uint32_t ret = 0;
	char* sptr;
	char* is;

	for (const char* i = format; *i != 0; i++){
		if (*i == '%'){
			i++;
			switch (*i){ 
				case 'd':
					sptr = itoa((int)va_arg(args, int), 10);
					for (is = sptr; *is != 0; is++){
						ret++;
						asm volatile ("pushad \n push 1 \n push %0 \n mov eax, esp \n int 0x30 \n add esp, 8 \n popad" : : "b"((uint32_t)*is) : "memory");
					}
					free(sptr);
					break;
				case 'i':
					sptr = itoa((int)va_arg(args, int), 36);
					for (is = sptr; *is != 0; is++){
						ret++;
						asm volatile ("pushad \n push 1 \n push %0 \n mov eax, esp \n int 0x30 \n add esp, 8 \n popad" : : "b"((uint32_t)*is) : "memory");
					}
					free(sptr);
					break;
				case 'o':
					sptr = itoa((int)va_arg(args, int), 8);
					for (is = sptr; *is != 0; is++){
						ret++;
						asm volatile ("pushad \n push 1 \n push %0 \n mov eax, esp \n int 0x30 \n add esp, 8 \n popad" : : "b"((uint32_t)*is) : "memory");
					}
					free(sptr);
					break;
				case 'h':
					sptr = itoa((int)va_arg(args, int), 16);
					for (is = sptr; *is != 0; is++){
						ret++;
						asm volatile ("pushad \n push 1 \n push %0 \n mov eax, esp \n int 0x30 \n add esp, 8 \n popad" : : "b"((uint32_t)*is) : "memory");
					}
					free(sptr);
					break;
				case 's':
					sptr = (char*)va_arg(args, char*);
					for (is = sptr; *is != 0; is++){
						ret++;
						asm volatile ("pushad \n push 1 \n push %0 \n mov eax, esp \n int 0x30 \n add esp, 8 \n popad" : : "b"((uint32_t)*is) : "memory");
					}
					break;
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
	char* ci;
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
				case 's':
					ci = (char*)(va_arg(args, char*));
					while (1){
						asm volatile ("pushad \n push 3 \n push 0 \n mov eax, esp \n int 0x30 \n add esp, 8 \n mov [esp-4], eax \n popad \n mov %0, dword [esp-40]"
							: "=b"(buf) : : "memory"); //NOTE: esp-40 Should actually be esp-36, but gcc subtracts 4 (still compiles to esp-36, esp-36 would compile to esp-32)
						if ((char)buf == ' ' || (char)buf == '\n'){
							if ((char)buf == (uint8_t)*(i+1) && (char)buf != '%') i++;
							break;
						}
						*ci = (char)buf;
						ret++;
						ci++;
					}

					*ci = 0;
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
#pragma GCC pop_options
#endif
