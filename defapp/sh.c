//sh.c
//
//Shell for AleemOS

#include "sh.h"
#include <stdio.h>

int main(){
	printf("AleemOS 0.0.0 x86_32\nPlease see the licensing for AleemOS at https://github.com/mebrahimaleem/AleemOS/blob/main/LICENSE\n\nWelcome to AleemOS Shell\n>");
	int cursorX = 1;
	int cursorY = 5;
	blink(cursorX, cursorY);
	while (1) asm volatile ("nop" : : : "memory");
	return 0;
}
