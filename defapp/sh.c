//sh.c
//
//Shell for AleemOS

#include "sh.h"
#include <stdio.h>
#include <AleemOS.h>

int main(int argc, char** argv){
	printf("AleemOS 0.0.0 x86_32\nPlease see the licensing for AleemOS at https://github.com/mebrahimaleem/AleemOS/blob/main/LICENSE\n\nWelcome to AleemOS Shell\n>");
	char c;
	blink(1, 5);
	while (1){
		//Get next
		scanf("%c", &c);
		blink(cursorPos(), 0);
	}
	return 0;
}
