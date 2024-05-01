//AleemOS.c

#ifndef __STDC_ALEEMOS_C
#define __STDC_ALEEMOS_C
#include <stdint.h>
#include <stdarg.h>
#include "stdlib.h"
#include "AleemOS.h"


#pragma GCC push_options
#pragma GCC optimize("O0")
int blink(int x, int y){
	_syscall(2, (uint32_t)(y * 80 + x));
	return 0;
}

int cursorPos(){
	return (int)_syscall(4, 0);
}
#pragma GCC pop_options

#endif
