//kernel.c
//
//This is the kernel for AleemOS

//Include freestanding libraries
#include <float.h>
#include <stdint.h>

//Include our own libraries
#include "basicio.h"
#include "portio.h"
#include "memory.h"

uint8_t k_DRIVE_NO;
uint16_t k_PARTITION_OFFSET;
uint16_t k_TRACK_SECTORS;
uint16_t k_HEADS;

void kernel(void){
	initHeap();
	
	volatile uint8_t* volatile loading_msg = "Loading ...";

	for (volatile uint8_t volatile* i = loading_msg; *i != 0; i++) put(*i, i-loading_msg, 1, 0x0F);

hang:
	asm volatile ("hlt");
goto hang;
}
