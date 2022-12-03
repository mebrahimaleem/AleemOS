//AleemOS.c

#ifndef __STDC_ALEEMOS_C
#define __STDC_ALEEMOS_C
#include <stdint.h>

int blink(int x, int y){
	asm volatile ("pushad \n push 2 \n push %0 \n mov eax, esp \n int 0x30 \n add esp, 8 \n popad" : : "b"((uint32_t)(y * 80 + x)) : "memory");
	return 0;
}

int cursorPos(){
	int ret;
	asm volatile ("pushad \n push 4 \n push 0 \n mov eax, esp \n int 0x30 \n add esp, 8 \n mov [esp-4], eax \n popad \n mov %0, dword [esp-40]"
		: "=b"((uint32_t)ret) : : "memory"); //NOTE: esp-40 Should actually be esp-36, but gcc subtracts 4 (still compiles to esp-36, esp-36 would compile to esp-32)
	return ret;
}
#endif
