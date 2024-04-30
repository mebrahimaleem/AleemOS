//process.c
//
//Implementations for process management

#define GET_PAGE_DIV(x) (uint8_t)((pageDivs[x/8] & (1 << (x%8))) >> x%8)
#define SET_PAGE_DIV(x) (pageDivs[x/8] = (uint8_t)(pageDivs[x/8] | (1 << (x%8))))
#define UNSET_PAGE_DIV(x) (pageDivs[x/8] = (uint8_t)(pageDivs[x/8] & (~(1 << (x%8)))))

#include <stdint.h>
#include <kernel.h>
#include <memory.h>
#include <basicio.h>
#include <ELFparse.h>
#include <signals.h>
#include <process.h>
#include <taskSwitch.h>
#include <processScheduler.h>

processState* processStack = 0;

uint8_t pageDivs[128];
uint32_t nextPID = 0;

#pragma GCC push_options
#pragma GCC optimize("O0")
//Start a process
void startProcess(processState* state){
	if ((state->priority & 0x80) == 0x80) {
		state->eax = state->argc;
		state->ebx = state->argv;
		state->ecx = state->HS;
		state->priority = state->priority & 0x7F;
	}
	taskSwitch((uint32_t)state);
}
#pragma GCC pop_options

//Kills the current process
void killProcess(){
	unscheduleCurrentProcess();
}

processSetup setupProcess(uint8_t* volatile src){
	processSetup ret;
	ret.res = 255;
	uint16_t i;

	//Find page divisions (2) for the process
	uint16_t pageDiv1 = 0, pageDiv2 = 0;
	for (i = 1; i < 1022; i++){
		if (GET_PAGE_DIV(i) == 1){
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

	UNSET_PAGE_DIV(pageDiv1);
	UNSET_PAGE_DIV(pageDiv2);

	ret.codeB = progB;
	ret.state.eax = 
		ret.state.ebx =
		ret.state.ecx =
		ret.state.edx =
		ret.state.esi =
		ret.state.edi = 0;

	ret.state.esp = 
		ret.state.ebp = 0xFFC00000 - 4; //subtract 0x14 to make room for the iret frame

	ret.state.eip = entry;
	ret.state.cr3 = pagingB;
	ret.state.eflags = 0x200;
	ret.state.HS = UHS;

	ret.state.PID = nextPID;
	nextPID++;

	addProcess(ret.state.PID);
	ret.res = 0;
	return ret;
}

void resetProcessDivs(){
	nextPID = 1;
	UNSET_PAGE_DIV(0);
	for (uint16_t i = 1; i < 1023; i++) SET_PAGE_DIV(i);
	return;
}
