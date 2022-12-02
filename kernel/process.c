//process.c
//
//Implementations for process management

#include <stdint.h>
#include "kernel.h"
#include "memory.h"
#include "process.h"

processState* processStack = 0;

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
	UTSS->ecx = state->ecx;
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
