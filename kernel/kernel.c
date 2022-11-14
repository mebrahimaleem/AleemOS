//kernel.c
//
//This is the kernel for AleemOS

//Include freestanding libraries
#include <float.h>
#include <stdint.h>

//Include our own libraries
#include "kernel.h"
#include "basicio.h"
#include "portio.h"
#include "memory.h"
#include "utils.h"
#include "../drivers/kbd.h"

volatile uint8_t k_DRIVE_NO;
volatile uint16_t k_PARTITION_OFFSET;
volatile uint16_t k_TRACK_SECTORS;
volatile uint16_t k_HEADS;

volatile uint16_t k_KDATA;

volatile KernelData* volatile kdata;

void kernel(void){
	initHeap();
	kdata = (volatile KernelData* volatile)(volatile uint32_t)k_KDATA;

	volatile const char* volatile loading_msg = "Loading ...";
	for(volatile const char* volatile i = loading_msg; *i != 0; i++) put(*i, (uint8_t)(i-loading_msg), 0, 0x0F);

	KBDResetMods();

hang:
	asm volatile ("hlt");
goto hang;
}
