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
						_syscall(1, (uint32_t)*is);
					}
					free(sptr);
					break;
				case 'i':
					sptr = itoa((int)va_arg(args, int), 36);
					for (is = sptr; *is != 0; is++){
						ret++;
						_syscall(1, (uint32_t)*is);
					}
					free(sptr);
					break;
				case 'o':
					sptr = itoa((int)va_arg(args, int), 8);
					for (is = sptr; *is != 0; is++){
						ret++;
						_syscall(1, (uint32_t)*is);
					}
					free(sptr);
					break;
				case 'h':
					sptr = itoa((int)va_arg(args, int), 16);
					for (is = sptr; *is != 0; is++){
						ret++;
						_syscall(1, (uint32_t)*is);
					}
					free(sptr);
					break;
				case 's':
					sptr = (char*)va_arg(args, char*);
					for (is = sptr; *is != 0; is++){
						ret++;
						_syscall(1, (uint32_t)*is);
					}
					break;
				case 'c':
					ret++;
					//NOTE: char is promoted to int when using ...
					_syscall(1, (uint32_t)va_arg(args, int));
					break;
				default: //Bad specifier
					return -1;
			}
		}

		else{
			ret++;
			_syscall(1, (uint32_t)*i);
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
					*(char*)(va_arg(args, char*)) = (char)_syscall(3, 0);
					break;
				case 's':
					ci = (char*)(va_arg(args, char*));
					while (1){
						buf = _syscall(3, 0);
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
			if ((uint8_t)_syscall(3, 0) != (uint8_t)*i) return -1;
			continue;
		}
	}

	_syscall(5, 0);
	return (int)ret;
}
#pragma GCC pop_options
#endif
