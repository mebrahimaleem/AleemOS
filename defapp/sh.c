//sh.c
//
//Shell for AleemOS

#include "sh.h"

int main(){
	while (1) asm volatile ("nop" : : : "memory");
	return 0;
}
