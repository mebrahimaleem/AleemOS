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
#include "signals.h"
#include "process.h"
#include "../drivers/pci.h"
#include "../drivers/kbd.h"
#include "ELFparse.h"

volatile uint8_t k_DRIVE_NO;
volatile uint16_t k_PARTITION_OFFSET;
volatile uint16_t k_TRACK_SECTORS;
volatile uint16_t k_HEADS;

volatile uint16_t k_KDATA;

volatile GDT_ptr k_gdtd;
volatile LDT_ptr k_ldtd;
volatile IDT_ptr k_idtd;
volatile TSS_ptr k_tssd;
volatile uint32_t k_CR3;
volatile uint32_t k_uidtd;
volatile uint32_t k_ugdtd;
volatile uint32_t k_uisrs;
volatile uint32_t k_uidts;

volatile KernelData* volatile kdata;

extern void setSysTables(void);

PCIEntry* pciEntries;

//Create new system tables	
TSS* volatile KTSS = (TSS* volatile)0xFFC06068;
TSS* volatile UTSS = (TSS* volatile)0xFFC060D0;
DTentry* volatile GDT = (DTentry* volatile)0xFFC06000;
IDTint* volatile IDT = (IDTint* volatile)0xFFC06200;

uint32_t defAppBP; 
uint32_t entry;

void kernel(void){
	initHeap(); //Setup kernel heap
	kdata = (volatile KernelData* volatile)(volatile uint32_t)k_KDATA; //Get kernel data
	clearVGA(); //Setup VGA

	//Display loading message
	vgaprint((volatile char* volatile)"Loading ...\n", 0x0F);

	//Setup keyboard
	KBDResetMods();

	//Setup signals
	initSignals();

	//Setup PCI
	pciEntries = getPCIDevices();
	for (PCIEntry* i = pciEntries; i != 0; i = i->next) {
		vgaprint((volatile char* volatile)"Found PCI Device. Bus: 0x", 0x0A);
		vgaprintint(i->bus, 16, 0x0A);
		vgaprint((volatile char* volatile)" Device: 0x", 0x0A);
		vgaprintint(i->dev, 16, 0x0A);
		vgaprint((volatile char* volatile)" Type: ", 0x0A);
		vgaprintint(i->type, 10, 0x0A);
		vgaprint((volatile char* volatile)" Func: ", 0x0A);
		vgaprintint(i->func, 10, 0x0A);
		vgaprint((volatile char* volatile)" Irq Line: 0x", 0x0A);
		vgaprintint((i->irqLine & 0xFF) + 0x20, 16, 0x0A);
		vgaprint((volatile char* volatile)"\n", 0x0A);
	}

	goto hang;
	
	//For commenting purposes, page divisions are the continous blocks of 4MB of RAM (that a PT defines)
	//First we need to add a new page table

	for (uint32_t* i = (uint32_t*)(0x400000 - 0x2000); i < (uint32_t*)(0x400000 - 0x1000); i++)
		*i = (((uint32_t)(i) + 0x2000 - 0x400000)/4 * 0x1000) | 3; //High kernel mapping, maps the final page division to the first physical memory block

	for (uint32_t* i = (uint32_t*)(0x400000 - 0x1000); i < (uint32_t*)0x400000; i++)
		*i = (((uint32_t)(i) + 0x1000 - 0x400000)/4 * 0x1000 + 0x400000) | 3; //Spare page table (later used when creating processes)

	*(uint32_t*)0xc004 = (0x400000 - 0x1000) | 3;
	*(uint32_t*)(0xd000-4) = (0x400000 - 0x2000) | 3;

	//Copy IDT and ISRs to system data page (0x6000)
	uint8_t* dest = (uint8_t*)0xFFC06200;
	for (uint8_t* i = (uint8_t*)(k_uidts); i < (uint8_t*)(k_uidts) + k_uisrs; i++){
		*dest = *i;
		dest++;
	}

	//Write system table descriptors to system data page
	*(GDT_ptr*)(0xFFC06138) = *(GDT_ptr*)k_ugdtd;
	*(IDT_ptr*)(0xFFC0613E) = *(IDT_ptr*)k_uidtd;

	//Setup process sections and paging structures	
	resetProcessDivs();
	processSetup defAppSetup = setupProcess((uint8_t*)0x800);

	if (defAppSetup.res != 0){
		vgaprint((volatile char* volatile)"ERROR [", 0x0F);
		vgaprintint(defAppSetup.res, 10, 0x0F);
		vgaprint((volatile char* volatile)"] : Bad Executable", 0x0F);

		goto hang;
	}
	
	//Create new system tables on the system data page
	//GDT
	//Null
	GDT[0].lim0 = 0;
	GDT[0].bas0 = 0;
	GDT[0].bas1 = 0;
	GDT[0].type = 0;
	GDT[0].DPL = 0;
	GDT[0].present = 0;
	GDT[0].lim1 = 0;
	GDT[0].AVL = 0;
	GDT[0].flg = 0;
	GDT[0].granularity = 0;
	GDT[0].bas2 = 0;

	//Code
	GDT[1].lim0 = 0xffff;
	GDT[1].bas0 = 0;
	GDT[1].bas1 = 0;
	GDT[1].type = 0x1a;
	GDT[1].DPL = 0;
	GDT[1].present = 1;
	GDT[1].lim1 = 15;
	GDT[1].AVL = 0;
	GDT[1].flg = 2;
	GDT[1].granularity = 1;
	GDT[1].bas2 = 0;

	//Data
	GDT[2].lim0 = 0xffff;
	GDT[2].bas0 = 0;
	GDT[2].bas1 = 0;
	GDT[2].type = 0x12;
	GDT[2].DPL = 0;
	GDT[2].present = 1;
	GDT[2].lim1 = 15;
	GDT[2].AVL = 0;
	GDT[2].flg = 2;
	GDT[2].granularity = 1;
	GDT[2].bas2 = 0;

	//User Code
	GDT[3].lim0 = 0xffff;
	GDT[3].bas0 = 0;
	GDT[3].bas1 = 0;
	GDT[3].type = 0x1a;
	GDT[3].DPL = 3;
	GDT[3].present = 1;
	GDT[3].lim1 = 15;
	GDT[3].AVL = 0;
	GDT[3].flg = 2;
	GDT[3].granularity = 1;
	GDT[3].bas2 = 0;

	//User Data
	GDT[4].lim0 = 0xffff;
	GDT[4].bas0 = 0;
	GDT[4].bas1 = 0;
	GDT[4].type = 0x12;
	GDT[4].DPL = 3;
	GDT[4].present = 1;
	GDT[4].lim1 = 15;
	GDT[4].AVL = 0;
	GDT[4].flg = 2;
	GDT[4].granularity = 1;
	GDT[4].bas2 = 0;

	//Kernel TSS
	GDT[5].lim0 = 0x68;
	GDT[5].bas0 = 0x6068;
	GDT[5].bas1 = 0xC0;
	GDT[5].type = 0x9;
	GDT[5].DPL = 0;
	GDT[5].present = 1;
	GDT[5].lim1 = 0;
	GDT[5].AVL = 0;
	GDT[5].flg = 2;
	GDT[5].granularity = 0;
	GDT[5].bas2 = 0xFF;
	
	//User TSS
	GDT[6].lim0 = 0x68;
	GDT[6].bas0 = 0x60D0;
	GDT[6].bas1 = 0xC0;
	GDT[6].type = 0xB;
	GDT[6].DPL = 3;
	GDT[6].present = 1;
	GDT[6].lim1 = 0;
	GDT[6].AVL = 0;
	GDT[6].flg = 2;
	GDT[6].granularity = 0;
	GDT[6].bas2 = 0xFF;
	
	//Kernel TSS
	KTSS->link = 48;
	KTSS->esp0 = 0xFFFFFFFC;
	KTSS->ss0 = 16;
	KTSS->esp1 = 0x7c00;
	KTSS->ss1 = 16;
	KTSS->esp2 = 0x7c00;
	KTSS->ss2 = 16;
	KTSS->cr3 = k_CR3;
	KTSS->eip = 0;
	KTSS->eflags = 0;
	KTSS->eax = 0;
	KTSS->ecx = 0;
	KTSS->edx = 0;
	KTSS->ebx = 0;
	KTSS->esp = 0xFFFFFFFC;
	KTSS->ebp = 0xFFFFFFFC;
	KTSS->esi = 0;
	KTSS->edi = 0;
	KTSS->es = 16;
	KTSS->cs = 8;
	KTSS->ss = 16;
	KTSS->ds = 16;
	KTSS->fs = 16;
	KTSS->gs = 16;
	KTSS->ldtd = 0;
	KTSS->IOPB = 0x68;
	
	*UTSS = defAppSetup.utss;

	//Setup system tables
	setSysTables();

	processState* defApp = &defAppSetup.state;
	defApp->IDN = 1;
	defApp->argc = 0; //Our first application does not care about the first argument (it already knows its /sh.elf) so don't pass it anything
	
	clearVGA();
	createProcess(defApp, 0);

hang:
	while (1) asm volatile ("hlt" : : : "memory");
}

#pragma GCC push_options
#pragma GCC optimize("O0")
uint32_t last_exitcode = 0;

//sysCall is meant for handling system calls from userland
uint32_t sysCall(uint32_t cs, uint32_t call, uint32_t params){
	uint32_t ret = 0;
	switch (call){
		case 0: //kill(exitcode)
			last_exitcode = params;
			killProcess();
			kernel();
			break;
		case 1: //printchar(char)
			vgaprintchar((uint8_t)params, 0x0F);
			params = (uint32_t)(vgacursor - (volatile uint8_t*)0xb8000)/2;
			backslock = (uint32_t)vgacursor;
			goto blink;
		blink:
		case 2: //blink(addr)
			outb(0x3D4, 0x0F);
			outb(0x3D5, (uint8_t)(params & 0xFF));
			outb(0x3D4, 0x0E);
			outb(0x3D5, (uint8_t)((params >> 8) & 0xFF));
			break;
		case 3: //getchar()
			break;
			setKBDEventTrack(1);
			while (1){
				if (KBDNextEvent != 0){
					ret = (uint32_t)toAscii(KBDNextEvent->keyCode);
					KBDNextEvent = KBDNextEvent->next;
					if (ret != 0){
						vgaprintchar((uint8_t)ret, 0x0F);
						params = (uint32_t)(vgacursor - (volatile uint8_t*)0xb8000)/2;
						setKBDEventTrack(0);
						goto blink;
					}
				}
				asm volatile ("sti \n hlt \n cli" : : : "memory");
				continue;
			}
		case 4: //getcursorpos()
			ret = (uint32_t)(vgacursor - (volatile uint8_t* volatile)0xb8000)/2;
			break;
		default:
			break;
	}

	//We need to execute a far return
	asm volatile ("mov eax, %0 \n leave \n retf" : : "m"(ret), "m"(cs) : "memory");
	return 0;
}

//processManager is meant for handling exception caused interupts, IRQs
void processManager(uint32_t cs, uint32_t check){
	//Check for kill process
	if (check == 0){
		last_exitcode = 1;
		killProcess();

		//If killProcess returned it means that it failed to start the previous process (probably because it doesn't exist)
		kernel();
	}

	//Check for IRQ0
	else if (check == 1) {
		kdata = (volatile KernelData* volatile)(volatile uint32_t)k_KDATA; //Get kernel data
		kdata->systemTime.fraction_ms += kdata->systemTime.fraction_diff;
		kdata->systemTime.whole_ms += kdata->systemTime.whole_diff;
		asm volatile ("push eax \n mov al, 0x20 \n out 0x20, al \n pop eax");
	}

	//Check for IRQ1
	else if (check == 2) {
		uint32_t stroke;
		asm volatile ("push eax \n in al, 0x60 \n movzx eax, al \n mov %0, eax \n pop eax" : "=m"(stroke): : "memory"); //Get keystroke
		ISR21_handler(stroke); //Pass keystroke to driver
		asm volatile ("push eax \n mov al, 0x20 \n out 0x20, al \n pop eax" : : : "memory"); //Tell PIC we are done
	}

	//Check for Master PIC IRQ
	else if (check <= 7){
		asm volatile ("push eax \n mov al, 0x20 \n out 0x20, al \n pop eax" : : : "memory"); //Tell PIC we are done
	}

	//Check for spurious
	else if (check == 8){
		uint32_t res;
		asm volatile ("push eax \n mov al, 0xb \n out 0x20, al \n xor al, al \n out 0x80, al \n in al, 0x20 \n movzx eax, al \n \
				mov %0, eax \n pop eax" : "=m"(res) : : "memory");
		if (res == 7)
			asm volatile ("push eax \n mov al, 0x20 \n out 0x20, al \n pop eax" : : : "memory");
	}

	//Check for Slave PIC IRQ
	else if (check <= 15){
		asm volatile ("push eax \n mov al, 0x20 \n out 0x20, al \n out 0xA0, al \n pop eax"); //Tell PIC we are done
	}

	//Check for spurious
	else if (check == 16){
		uint32_t res;
		asm volatile ("push eax \n mov al, 0xb \n out 0xA0, al \n xor al, al \n out 0x80, al \n in al, 0xA0 \n movzx eax, al \n \
				mov %0, eax \n pop eax" : "=m"(res) : : "memory");
		if (res == 15)
			asm volatile ("push eax \n mov al, 0x20 \n out 0xA0, al \n pop eax" : : : "memory");
		asm volatile ("push eax \n mov al, 0x20 \n out 0x20, al \n pop eax" : : : "memory");
	}

	//We need to execute a far return
	asm volatile ("leave \n retf" : : "m"(cs) : "memory");
}
#pragma GCC pop_options
