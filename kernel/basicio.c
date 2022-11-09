//basicio.c
//
//IO library for the kernel using VGA

#include <stdint.h>
#include "basicio.h" 

extern inline void put(volatile char str, volatile uint8_t x, volatile uint8_t y, volatile uint8_t color){
	*(volatile char* volatile)(0xb8000 + (x + y * 80)*2) = str;
	*(volatile uint8_t* volatile)(0xb8000 + (x + y * 80)*2 + 1) = color;
	return;
}
