//portio.c
//
//Implementations for IO port interacations
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


#include <stdint.h>
#include <portio.h>

#pragma GCC push_options
#pragma GCC optimize("O0")
inline void outb(volatile uint16_t port, volatile uint8_t byte){
	asm volatile ("out %1, %0" : : "a"(byte), "Nd"(port) : "memory");
	return;
}

inline void outw(volatile uint16_t port, volatile uint16_t word){
	asm volatile ("out %1, %0" : : "a"(word), "Nd"(port) : "memory");
	return;
}

inline void outd(volatile uint16_t port, volatile uint32_t dword){
	asm volatile ("out %1, %0" : : "a"(dword), "Nd"(port));
	return;
}

inline uint8_t inb(volatile uint16_t port){
	uint8_t byte;
	asm volatile ("in %0, %1" : "=a"(byte) : "Nd"(port) : "memory");
	return byte;
}

inline uint16_t inw(volatile uint16_t port){
	uint16_t word;
	asm volatile ("in %0, %1" : "=a"(word) : "Nd"(port) : "memory");
	return word;
}

inline uint32_t ind(volatile uint16_t port){
	uint32_t dword;
	asm volatile ("in %0, %1" : "=a"(dword) : "Nd"(port) : "memory");
	return dword;
}

inline void io_wait(void){
	outb(0x80, 0);
	return;
}
#pragma GCC pop_options
