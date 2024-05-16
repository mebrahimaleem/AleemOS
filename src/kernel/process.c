//process.c
//
//Implementations for process management

#include <stdint.h>
#include <kernel.h>
#include <memory.h>
#include <utils.h>
#include <basicio.h>
#include <paging.h>
#include <ELFparse.h>
#include <signals.h>
#include <process.h>
#include <taskSwitch.h>
#include <processScheduler.h>

uint32_t nextProcessVaddr;
uint32_t nextPID;

#pragma GCC push_options
#pragma GCC optimize("O0")
//Start a process
void startProcess(processState* state){
	if ((state->priority & 0x80) == 0x80) {
		state->eax = state->argc;
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

processSetup setupProcess(uint8_t* src, uint8_t priority, uint32_t argc, uint8_t** argv){
	processSetup* ret = (processSetup*)malloc(sizeof(processSetup));
	ret->res = 255;
	
	mapMemory4M(kernelPD, nextProcessVaddr, nextProcessVaddr, 3);
	
	ElfPageList* pages;
	uint32_t sz;
	uint32_t entry;

	//Get memory layout information from the executable
	if (calcELF(src, &pages, &sz) != 0){
		ret->res = 2; //Bad ELF
		return *ret;
	}

	//Copy executable
	if (copyELF(src, (uint8_t*)nextProcessVaddr, &entry) != 0){ //Copy the file after we know the paging layout
		ret->res = 2; //Bad ELF
		return *ret;
	}

	uint32_t* UPD = allocPagingStruct();
	uint32_t UHS = 0;

	for (ElfPageList* p = pages; p != 0; p = p->next) {
		mapMemory((uint32_t***)UPD, p->vaddr, nextProcessVaddr + p->paddr, 7);

		// check if heap will conflict with program section
		if (UHS < p->paddr + 0x1000) UHS = p->paddr + 0x1000;
	}

	mapMemory4M((uint32_t***)UPD, 0xff800000, nextProcessVaddr, 7); // map heap and stack
	mapMemory4M((uint32_t***)UPD, 0xffc00000, 0x00000000, 5); // map high kernel for interrupts only

	ret->state.kHeapVaddr = nextProcessVaddr + UHS;

	// pass argv via heap
	if (argc != 0) {
		*(uint32_t* )(nextProcessVaddr + UHS) = 0;
		const char** argv_b = (const char**)_malloc(argc * sizeof(char*), (BlockDescriptor*)(nextProcessVaddr + UHS));
		
		uint32_t nextStr = 0xff800000 + + UHS + 4 + argc * sizeof(char*);
		for (uint32_t i = 0; i < argc; i++) {
			const uint32_t strLen = strlen(argv[i]) + 1;
			strcpy((uint8_t*)_malloc(strLen, (BlockDescriptor*)(nextProcessVaddr + UHS)), argv[i]);
			argv_b[i] = (char*)(nextStr + 4);
			nextStr += 4 + strLen;
		}
	}

	nextProcessVaddr += 0x00400000;

	ret->state.eax = 
		ret->state.ebx =
		ret->state.ecx =
		ret->state.edx =
		ret->state.esi =
		ret->state.edi = 0;

	ret->state.esp = 
		ret->state.ebp = 0xFFC00000 - 4;

	ret->state.eip = entry;
	ret->state.cr3 = (uint32_t)UPD;
	ret->state.eflags = 0x200;

	// move heap under the stack
	UHS += 0xff800000;
	ret->state.HS = UHS;

	ret->state.PID = nextPID;
	nextPID++;

	ret->state.priority = priority | 0x80;

	ret->state.argc = argc;

	addProcess(ret->state.PID);
	ret->res = 0;
	return *ret;
}

void resetProcessDivs(){
	nextPID = 1;
	nextProcessVaddr = PROCESS_VADDR_BASE;
	return;
}
