#include <stdint.h>
#include <memory.h>
#include <paging.h>

#define PD_at(x) ((uint32_t**)((uint32_t)PD[PD_i] & 0xfffff000))

uint8_t* volatile nextPagingBase;
uint32_t*** kernelPD;

void initPaging(){
	nextPagingBase = (uint32_t* volatile)0x3ff000;
	kernelPD = (uint32_t***)0xc000;
}

uint8_t mapMemory(uint32_t*** PD, uint32_t vaddr, uint32_t paddr, uint8_t flg) {
	const uint32_t PD_i = vaddr >> 0x16;
	const uint32_t PT_i = (vaddr & 0x3ff000) >> 0xc;

	if (((uint32_t)PD[PD_i] & 1) == 0) // no page table assigned 
		PD[PD_i] = (uint32_t**)((uint32_t)allocPagingStruct() | (uint32_t)flg);

	if (((uint32_t)PD[PD_i][PT_i] & 1) == 0) // page division is free
		PD_at(PD_i)[PT_i] = (uint32_t*)(paddr | flg);

	else return 1; // Memory already mapped

	return 0;
}

uint8_t mapMemory4M(uint32_t*** PD, uint32_t vaddr, uint32_t paddr, uint8_t flg) {
	const uint32_t PD_i = vaddr >> 0x16;

	if (((uint32_t)PD[PD_i] & 1) == 0) // no page table assigned
		for (uint32_t i = 0; i < 0x400000; i += 0x1000) mapMemory(PD, vaddr + i, paddr + i, flg);

	else return 1; // Memory already mapped

	return 0;
}

uint32_t* allocPagingStruct() {
	nextPagingBase -= 0x1000;
	for (uint8_t* i = nextPagingBase; i < nextPagingBase + 0x1000; i++) *i = 0;
	return (uint32_t*)nextPagingBase;
}
