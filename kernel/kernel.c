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

uint16_t k_KDATA;

typedef struct MemoryMapEntry {
	uint64_t base;
	uint64_t size;
	uint32_t type;
} MemoryMapEntry;

typedef struct KernelData {
	uint32_t MBR_SIG;
	MemoryMapEntry mmape01;
	MemoryMapEntry mmape02;
	MemoryMapEntry mmape03;
	MemoryMapEntry mmape04;
	MemoryMapEntry mmape05;
	MemoryMapEntry mmape06;
	MemoryMapEntry mmape07;
	MemoryMapEntry mmape08;
	MemoryMapEntry mmape09;
	MemoryMapEntry mmape10;
	MemoryMapEntry mmape11;
	MemoryMapEntry mmape12;
	MemoryMapEntry mmape13;
	MemoryMapEntry mmape14;
	MemoryMapEntry mmape15;
	MemoryMapEntry mmape16;
	MemoryMapEntry mmape17;
	MemoryMapEntry mmape18;
	MemoryMapEntry mmape19;
	MemoryMapEntry mmape20;
} KernelData;

void kernel(void);

void kernel(void){
	initHeap();

	volatile const char* volatile loading_msg = "Loading ...";
	for(volatile const char* volatile i = loading_msg; *i != 0; i++) put(*i, (uint8_t)(i-loading_msg), 0, 0x0F);
	
hang:
	asm volatile ("hlt");
goto hang;
}
