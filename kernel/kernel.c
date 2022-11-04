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

typedef struct KernelData{
	uint16_t memamp;
} KernelData;

KernelData k_KDATA;

void kernel(void);

void kernel(void){
	initHeap();

	volatile const char* volatile loading_msg = "Loading ...";
	for(volatile const char* volatile i = loading_msg; *i != 0; i++) put(*i, (uint8_t)(i-loading_msg), 0, 0x0F);
	
hang:
	asm volatile ("hlt");
goto hang;
}
