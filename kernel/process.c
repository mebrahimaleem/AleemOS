//process.c
//
//Implementations for process management

#include <stdint.h>
#include "kernel.h"
#include "memory.h"
#include "basicio.h"
#include "ELFparse.h"
#include "process.h"

processState* processStack = 0;

bool1 pageDivs[1024];

#pragma GCC push_options
#pragma GCC optimize("O0")
//Start a process
void startProcess(processState* state, uint8_t toStart){
	GDT[5].type = 0x9;
	GDT[6].type = 0xB;
	KTSS->link = 48;
	UTSS->cr3 = state->cr3;
	UTSS->eax = (toStart == 1 ? state->argc : state->eax);
	UTSS->ebx = (toStart == 1 ? state->argv : state->ebx);
	UTSS->ecx = (toStart == 1 ? state->HS : state->ecx);
	UTSS->edx = state->edx;
	UTSS->esi = state->esi;
	UTSS->edi = state->edi;
	UTSS->esp = state->esp;
	UTSS->ebp = state->ebp;
	UTSS->eip = state->eip;
	UTSS->eflags = state->eflags;
	
	asm volatile ("pushf \n pop ecx \n and ecx, 0xFFC0802A \n or ecx, 0x4200 \n mov %1, ecx \n \
			mov ax, 0x23 \n mov dx, ax \n mov es, ax \n mov fs, ax \n mov gs, ax \n \
		 	mov eax, esp \n push 0x23 \n push eax \n push ecx \n push 0x1b \n push %0 \n iret" :  "+b"(state->eip) : "m"(state->eflags): "memory");
}
#pragma GCC pop_options


//Creates and starts a new process with the processState 'state'
void createProcess(processState* state, processState* cstate){
	if (processStack == 0) processStack = state;
	else {
		state->next = cstate;
		cstate->next = processStack->next;
		processStack = state;
	}
	startProcess(processStack, 1);	
}

//Kills the current, returns 0 if no processes remain
uint32_t killProcess(){
	if (processStack == 0) return 0;
	processState* tmp = processStack;
	processStack = processStack->next;
	free(tmp);
	if (processStack == 0) return 0;

	startProcess(processStack, 0);
	return 1;
}

processSetup setupProcess(uint8_t* volatile src){
	processSetup ret;
	ret.res = 255;
	uint16_t i;

	//Find page divisions (2) for the process
	uint16_t pageDiv1 = 0, pageDiv2 = 0;
	for (i = 1; i < 1023; i++){
		if (pageDivs[i].b == 1){
			if (pageDiv1 == 0){
				pageDiv1 = i;
				continue;
			}
			pageDiv2 = i;
			break;
		}
	}
	
	//Check if we got page divisions
	if (pageDiv2 == 0){
		ret.res = 1; //Out of memory
		return ret;
	}

	const uint32_t pagingB = pageDiv1 * 0x400000;
	const uint32_t progB = pageDiv2 * 0x400000;

	//Create update kernel paging structure to map executable
	uint32_t* PD = (uint32_t*)0xc000;
	uint32_t* PT = (uint32_t*)(0x400000 - 0x1000);	
	PD[pageDiv1] = (0x400000 - 0x1000) | 3;
	PD[pageDiv2] = (0x400000 - 0x1000) | 3;

	for (i = 0; i < 1024; i++)
		PT[i] = (progB + i * 0x1000) | 3;
	
	ElfPageList* pages;
	uint32_t sz;
	uint32_t entry;

	//Get memory layout information from the executable
	if (calcELF(src, &pages, &sz) != 0){
		ret.res = 2; //Bad ELF
		return ret;
	}

	//Copy executable
	if (copyELF(src, (uint8_t*)progB, &entry) != 0){ //Copy the file after we know the paging layout
		ret.res = 2; //Bad ELF
		return ret;
	}


	for (i = 0; i < 1024; i++)
		PT[i] = (pagingB + i * 0x1000) | 3;
	
	uint32_t* UPD = (uint32_t*)pagingB;
	uint32_t* UPT1 = (uint32_t*)(pagingB + 0x1000);
	uint32_t* UPT3 = (uint32_t*)(pagingB + 0x2000);

	uint32_t* UPT2;

	UPD[1022] = (pagingB + 0x1000) | 7;
	UPD[1023] = (pagingB + 0x2000) | 5;

	for (i = 0; i < 1022; i++) UPD[i] = 0;

	for (i = 0; i < 1024; i++){
		UPT1[i] = (pagingB + i * 0x1000) | 7;
		UPT3[i] = (uint32_t)((i * 0x1000) | 5);
	}
	
	UPT1[0] = UPT1[1] = UPT1[2] = 0; //Unmap PDs and PTs for security

	uint32_t UHS = pagingB + 0x3000;
	for (ElfPageList* p = pages; p->next != 0; p = p->next){
		const uint32_t PDindex = p->vaddr / 0x400000;
		const uint32_t PTindex = (p->vaddr - PDindex) / 0x1000;
		if (UPD[PDindex] == 0){
			UPD[PDindex] = UHS | 7;
			UHS += 0x1000;
		}
		
		UPT2 = (uint32_t*)(UPD[PDindex] - 7);
		UPT2[PTindex] = (progB + p->paddr) | 7;
	}

	UHS -= pagingB;
	UHS += 0xFF800000;

	pageDivs[pageDiv1].b = 0;
	pageDivs[pageDiv2].b = 0;

	ret.codeB = progB;
	ret.state.eax = 
		ret.state.ebx =
		ret.state.ecx =
		ret.state.edx =
		ret.state.esi =
		ret.state.edi = 0;

	ret.state.esp = 
		ret.state.ebp = 0xFFC00000 - 4;

	ret.state.eip = entry;
	ret.state.cr3 = pagingB;
	ret.state.eflags = 0x200;
	ret.state.HS = UHS;

	ret.utss.link = 48;
	ret.utss.esp0 = 0xFFFFFFFc;
	ret.utss.esp1 =
		ret.utss.esp2 = 0x7c00;
	ret.utss.ss0 =
		ret.utss.ss1 =
		ret.utss.ss2 = 16;
	ret.utss.cr3 = pagingB;
	ret.utss.eip = entry;
	ret.utss.eflags = 0x200;
	ret.utss.eax =
		ret.utss.ecx =
		ret.utss.edx =
		ret.utss.ebx =
		ret.utss.esi =
		ret.utss.edi =
		ret.utss.ldtd = 0;
	ret.utss.esp =
		ret.utss.ebp = 0xFFC00000 - 4;
	ret.utss.es =
		ret.utss.ss =
		ret.utss.ds =
		ret.utss.fs =
		ret.utss.gs = 32 | 3;
	ret.utss.cs = 24 | 3;
	ret.utss.IOPB = 0x68;

	ret.res = 0;
	return ret;
}

void resetProcessDivs(){
	pageDivs[0].b = 0;
	for (uint16_t i = 1; i < 1023; i++) pageDivs[i].b = 1;
}
