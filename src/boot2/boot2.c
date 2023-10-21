//kernel.c
//
//This is the kernel for AleemOS

//Include freestanding libraries
#include <float.h>
#include <stdint.h>

//Include our own libraries
#include <boot2.h>
#include <kernel.h>
#include <basicio.h>
#include <portio.h>
#include <memory.h>
#include <utils.h>
#include <signals.h>
#include <process.h>
#include <pci.h>
#include <xhci.h>
#include <kbd.h>
#include <ELFparse.h>

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

void boot2(void){
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
	
	//Setup XHCI
	initXHCIDriver();
	
	//For commenting purposes, page divisions are the continous blocks of 4MB of RAM (that a PT defines)
	//First we need to add a new page table

	for (uint32_t* i = (uint32_t*)(0x400000 - 0x2000); i < (uint32_t*)(0x400000 - 0x1000); i++)
		*i = (((uint32_t)(i) + 0x2000 - 0x400000)/4 * 0x1000) | 3; //High kernel mapping, maps the final page division to the first physical memory block

	for (uint32_t* i = (uint32_t*)(0x400000 - 0x1000); i < (uint32_t*)0x400000; i++)
		*i = (((uint32_t)(i) + 0x1000 - 0x400000)/4 * 0x1000 + 0x400000) | 3; //Spare page table (later used when creating processes)

	for (uint32_t* i = (uint32_t*)(0x400000 - 0x3000); i < (uint32_t*)(0x400000 - 0x2000); i++)
		*i = 0; // Page table for drivers to use (to access IO Mapping Addresses)

	*(uint32_t*)0xc004 = (0x400000 - 0x1000) | 3; // Second page division
	*(uint32_t*)(0xd000-4) = (0x400000 - 0x2000) | 3; //Last page division
	*(uint32_t*)(0xd000-8) = (0x400000 - 0x3000) | 3; // Second last Page division

	uint16_t nextPT = 0;

	for (PCIEntry* i = pciEntries; i != 0; i = i->next) {
		if (i->type == USB_XHCI) {
			nextPT = setupXHCIDevice(*i, nextPT);
		}
	}

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

#ifdef KERNEL_DEBUG
	vgaprint((volatile char* volatile)"Press Any Key To Continue...\n", 0x0F);

	setKBDEventTrack(1);
	while (1){
		if (KBDNextEvent != 0){
			uint32_t v = (uint32_t)toAscii(KBDNextEvent->keyCode);
			KBDNextEvent = KBDNextEvent->next;
			if (v != 0){
				setKBDEventTrack(0);
				break;
			}
		}
	}
#endif
	
	// wait for key press

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

void transferFromBoot2() {
 //TODO: implement
}
