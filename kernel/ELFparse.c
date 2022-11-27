//ELFparse.c
//
//This file provides implementations for reading ELF files

#include <stdint.h>
#include "memory.h"
#include "utils.h"
#include "ELFparse.h"

inline uint8_t parseElf(uint8_t* volatile src, Elf32sheader** shead, Elf32pheader** phead, Elf32Half* snum, Elf32Half* pnum, uint8_t** strtbl, uint32_t* entry){
	Elf32header* volatile ehead = (Elf32header*)src;

	if (ehead->indent.fid[0] != 0x7f || ehead->indent.fid[1] != 'E' || ehead->indent.fid[2] != 'L' || ehead->indent.fid[3] != 'F' || 
			ehead->indent.cls != 1 || ehead->indent.encd != 1 || ehead->indent.ver != 1 ||
			ehead->type != 2 || ehead->machine != 3 || ehead->version != 1) return 1; //Bad ELF
	
	*shead = (Elf32sheader* volatile)(ehead->shoff + (uint32_t)src);
	*phead = (Elf32pheader* volatile)(ehead->phoff + (uint32_t)src);

	*strtbl = (uint8_t* volatile)(shead[0][ehead->shstrndx].offs + (uint32_t)src);

	*snum = ehead->shnum;
	*pnum = ehead->phnum;
	*entry = ehead->entry;

	return 0;
}

uint8_t calcELF(uint8_t* volatile src, ElfPageList** pages, uint32_t* size){
	Elf32sheader* shead;
	Elf32pheader* phead;
	uint8_t* strtbl;
	Elf32Half snum = 0;
	Elf32Half pnum = 0;
	uint32_t entry = 0;

	if (parseElf(src, &shead, &phead, &snum, &pnum, &strtbl, &entry) != 0) return 1;

	*pages = malloc(sizeof(ElfPageList));
	ElfPageList* last = *pages;
	last->next = 0;
	
	uint32_t maxs;
	*size = 0;
	
	if (pnum == 0) return 3;
	for (Elf32Half i = 0; i < pnum; i++)
		if (phead[i].type == 1){ 
			if (phead[i].offs % 0x1000 != 0 || phead[i].vaddr % 0x1000 != 0) return 2; //ELF program not page aligned
			for (Elf32Word j = 0; j < phead[i].msize; j += 0x1000){
				last->paddr = phead[i].offs + j * 0x1000;
				last->vaddr = phead[i].vaddr + j * 0x1000;
				last->next = malloc(sizeof(ElfPageList));
				last = last->next;
				last->next = 0;
			}

			maxs = phead[i].vaddr + phead[i].msize; 
			*size = (*size > maxs ? *size : maxs);
		}
	
	if (*size == 0) return 3;
	return 0;
}

#include "basicio.h"
uint8_t copyELF(uint8_t* volatile src, uint8_t* volatile dst, uint32_t* volatile ent){
	Elf32sheader* shead;
	Elf32pheader* phead;
	uint8_t* strtbl;
	Elf32Half snum;
	Elf32Half pnum;
	uint32_t entry;

	if (parseElf(src, &shead, &phead, &snum, &pnum, &strtbl, &entry) != 0) return 1;

	uint32_t sigA;
	uint32_t trnI;

	*ent = entry;
	for (Elf32Half i = 0; i < snum; i++){
		if (strcmp((uint8_t*)".text", (uint8_t*)((uint32_t)strtbl + shead[i].name)) == 1 || 
				strcmp((uint8_t*)".data", (uint8_t*)((uint32_t)strtbl + shead[i].name)) == 1 || 
				strcmp((uint8_t*)".rodata", (uint8_t*)((uint32_t)strtbl + shead[i].name)) == 1)
			for (Elf32Word j = 0; j < shead[i].size; j++){
				sigA = (uint32_t)(dst + shead[i].addr + j - 0x3000) & 0xFFF;
				trnI = (uint32_t)(dst + shead[i].addr + j - 0x3000) & 0x3FF000;
				trnI /= 0x1000; //Get index in PT
				sigA += (*(uint32_t*)(0x401000 + trnI * 4) & 0x3FF000) + (uint32_t)dst - 0x3000;
				*(uint8_t*)sigA = *(src + shead[i].offs + j);
			}

		else if (strcmp((uint8_t*)".bss", (uint8_t*)(uint32_t)strtbl[shead[i].name]) == 1)
			for (Elf32Word j = 0; j < shead[i].size; j++){
				sigA = (uint32_t)(dst + shead[i].addr + j - 0x3000) & 0xFFF;
				trnI = (uint32_t)(dst + shead[i].addr + j - 0x3000) & 0x3FF000;
				trnI /= 0x1000; //Get index in PT
				sigA += (*(uint32_t*)(0x401000 + trnI * 4) & 0x3FF000) + (uint32_t)dst - 0x3000;
				*(uint8_t*)sigA = 0;
			}
	}

	return 0;
}
