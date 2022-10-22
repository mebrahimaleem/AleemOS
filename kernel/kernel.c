//kernel.c
//
//This is the kernel for AleemOS

//Include freestanding libraries
#include <float.h>
#include <stdint.h>

//Include our own libraries
#include "basicio.h"

uint8_t k_DRIVE_NO;
uint16_t k_PARTITION_OFFSET;
uint16_t k_TRACK_SECTORS;
uint16_t k_HEADS;

void kernel(void){
	put('X', 1, 1);
	while (1);
}
