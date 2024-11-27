//boot2.c
//
//This is the second stage bootloader for AleemOS
/*
MIT License

Copyright 2022-2024 Ebrahim Aleem

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
*/

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
#include <paging.h>
#include <process.h>
#include <processScheduler.h>
#include <pci.h>
#include <xhci.h>
#include <kbd.h>
#include <ELFparse.h>

uint8_t k_DRIVE_NO;
uint16_t k_PARTITION_OFFSET;
uint16_t k_TRACK_SECTORS;
uint16_t k_HEADS;

uint16_t k_KDATA;

GDT_ptr* k_gdtd;
IDT_ptr* k_idtd;
uint32_t k_CR3;
uint32_t k_uidtd;
uint32_t k_uisrs;
uint32_t k_uidts;

KernelData* kdata;

PCIEntry* pciEntries;

//Create new system tables	
DTentry* GDT;
IDTint* IDT = (IDTint* )0xFFC06200;
TSS* kTSS = 0;

uint32_t defAppBP; 
uint32_t entry;
processState* defApp;

void boot2(void){
	initHeap(); //Setup kernel heap
	kdata = (KernelData* )(uint32_t)k_KDATA; //Get kernel data
	clearVGA(); //Setup VGA

	//Display loading message
	vgaprint((char* )"Loading ...\n", 0x0F);

	//Setup keyboard
	KBDResetMods();

	initPaging(); // Setup advanced paging utilities

	// Start process scheduling
	resetProcessDivs();
	initScheduler();
	schedulerStatus = 0;

	//Setup PCI
	pciEntries = getPCIDevices();
	
	//Setup XHCI
	initXHCIDriver();
	
	uint32_t nextDriverVaddr = DRIVER_VADDR_BASE;

	for (PCIEntry* i = pciEntries; i != 0; i = i->next) {
		if (i->type == USB_XHCI) {
			nextDriverVaddr = setupXHCIDevice(*i, nextDriverVaddr);
		}
	}

	//Copy IDT and ISRs to system data page (0x6000)
	uint8_t* dest = (uint8_t*)0xFFC06200;
	for (uint8_t* i = (uint8_t*)(k_uidts); i < (uint8_t*)(k_uidts) + k_uisrs; i++){
		*dest = *i;
		dest++;
	}

	//Write system table descriptors to system data page
	*(GDT_ptr* )(0xFFC06138) = *k_gdtd;
	*(IDT_ptr* )(0xFFC0613E) = *(IDT_ptr*)k_uidtd;
	GDT = (DTentry*)0xBFD0;

	//Setup process sections and paging structures	
	const char** const procArgv = (const char**)malloc(2 * sizeof(char*));
	const char* argv0 = "/sh.elf"; //file name
	const char* argv1 = "/"; //starting directory
	procArgv[0] = argv0;
	procArgv[1] = argv1;

	processSetup defAppSetup = setupProcess((uint8_t*)0x1800, 1, 2, (uint8_t**)procArgv); //min.c has already loaded defapp from fs

	if (defAppSetup.res != 0){
		vgaprint("ERROR [", 0x04);
		vgaprintint(defAppSetup.res, 10, 0x04);
		vgaprint("] : Bad Executable", 0x04);

		hang();
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
	// wait for key press
	vgaprint("Press Any Key To Continue...\n", 0x0F);

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
	
	defApp = &defAppSetup.state;

	clearVGA();
	setSysTables();
	transferFromBoot2();

}

void transferFromBoot2() {
	kTSS->ss0 = 2 * 8;
	kTSS->esp0 = 0xFFFFFFFC;

	//clearVGA();
	scheduleProcess(defApp);

	asm volatile ("sti" : : : "memory");
	
	killProcess(_schedulerCurrentProcess->PID);

	hang();
}

void hang() {
	while (1) asm volatile ("hlt" : : : "memory");
}
