//boot2.c
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

volatile GDT_ptr* k_gdtd;
volatile IDT_ptr* k_idtd;
volatile uint32_t k_CR3;
volatile uint32_t k_uidtd;
volatile uint32_t k_uisrs;
volatile uint32_t k_uidts;

volatile KernelData* volatile kdata;

PCIEntry* pciEntries;

//Create new system tables	
DTentry* volatile GDT;
IDTint* volatile IDT = (IDTint* volatile)0xFFC06200;
volatile TSS* volatile kTSS = 0;

uint32_t defAppBP; 
uint32_t entry;
processState* defApp;

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
	*(GDT_ptr* volatile)(0xFFC06138) = *k_gdtd;
	*(IDT_ptr* volatile)(0xFFC0613E) = *(IDT_ptr*)k_uidtd;
	GDT = (DTentry*)0xBFD0;

	//Setup process sections and paging structures	
	resetProcessDivs();
	processSetup defAppSetup = setupProcess((uint8_t*)0x800);

	if (defAppSetup.res != 0){
		vgaprint((volatile char* volatile)"ERROR [", 0x0F);
		vgaprintint(defAppSetup.res, 10, 0x0F);
		vgaprint((volatile char* volatile)"] : Bad Executable", 0x0F);

		goto hang;
	}
	
	//Install User Data and Code Segments
	//GDT
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

	kTSS = (TSS*)((uint32_t)kTSS + 0xffc00000); // Move to high kernel

	//TSS
	GDT[5].lim0 = 0x68;
	GDT[5].bas0 = (uint16_t)(uint32_t)kTSS;
	GDT[5].bas1 = (uint8_t)((uint32_t)kTSS >> 16);
	GDT[5].type = 0x9;
	GDT[5].DPL = 0;
	GDT[5].present = 1;
	GDT[5].lim1 = 0;
	GDT[5].AVL = 0;
	GDT[5].flg = 2;
	GDT[5].granularity = 0;
	GDT[5].bas2 = (uint8_t)((uint32_t)kTSS >> 24);

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

	defApp = &defAppSetup.state;
	defApp->IDN = 1;
	defApp->argc = 0; //Our first application does not care about the first argument (it already knows its /sh.elf) so don't pass it anything

	transferFromBoot2();

hang:
	while (1) asm volatile ("hlt" : : : "memory");
}

void transferFromBoot2() {
	kTSS->ss0 = 2 * 8;
	kTSS->esp0 = 0xFFFFFFFC;

	clearVGA();
	createProcess(defApp, 0);
}
