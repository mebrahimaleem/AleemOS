//portio.c
//
//Implementations for IO port interacations


#include <stdint.h>
#include "portio.h"

extern inline void outb(volatile uint16_t port, volatile uint8_t byte){
	asm volatile ("out %1, %0" : : "a"(byte), "Nd"(port));
	return;
}

extern inline void outw(volatile uint16_t port, volatile uint16_t word){
	asm volatile ("out %1, %0" : : "a"(word), "Nd"(port));
	return;
}

extern inline void outd(volatile uint16_t port, volatile uint32_t dword){
	asm volatile ("out %1, %0" : : "a"(dword), "Nd"(port));
	return;
}

extern inline volatile uint8_t inb(volatile uint16_t port){
	uint8_t byte;
	asm volatile ("in %0, %1" : "=a"(byte) : "Nd"(port));
	return byte;
}

extern inline volatile uint16_t inw(volatile uint16_t port){
	uint16_t word;
	asm volatile ("in %0, %1" : "=a"(word) : "Nd"(port));
	return word;
}

extern inline volatile uint32_t ind(volatile uint16_t port){
	uint8_t dword;
	asm volatile ("in %0, %1" : "=a"(dword) : "Nd"(port));
	return dword;
}

extern inline void io_wait(void){
	outb(0x80, 0);
	return;
}
