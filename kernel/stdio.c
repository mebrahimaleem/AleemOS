//stdio.c
//
//IO library for the kernel using VGA

#include "types.h"

extern inline void put(volatile char str, volatile uint8_t x, volatile uint8_t y){
	*(volatile char* volatile)(0xb8000 + (x + y * 80)*2) = str;

	return;
}
