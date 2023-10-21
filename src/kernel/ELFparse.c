//ELFparse.c
//
//This file provides implementations for reading ELF files

#include <stdint.h>
#include <memory.h>
#include <utils.h>
#include <ELFparse.h>

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
				last->paddr = j;
				last->vaddr = phead[i].vaddr + j;
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

uint8_t copyELF(uint8_t* volatile src, uint8_t* volatile dst, uint32_t* volatile ent){
	Elf32sheader* shead;
	Elf32pheader* phead;
	uint8_t* strtbl;
	Elf32Half snum;
	Elf32Half pnum;
	uint32_t entry;

	if (parseElf(src, &shead, &phead, &snum, &pnum, &strtbl, &entry) != 0) return 1;

	*ent = entry;
	for (Elf32Half i = 0; i < snum; i++){
		if (shead[i].type == 1 && (shead[i].flags & 2) == 2) //PROGBITS
			for (Elf32Word j = 0; j < shead[i].size; j++){
				*(uint8_t*)(j + dst + shead[i].addr) = *(src + shead[i].offs + j);
			}

		else if (shead[i].type == 8 && (shead[i].flags & 2) == 2) //NOBITS
			for (Elf32Word j = 0; j < shead[i].size; j++){
				*(uint8_t*)(j + dst + shead[i].addr) = 0;
			}
	}

	return 0;
}
