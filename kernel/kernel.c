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

//Create new system tables	
	DTentry* volatile GDT = (DTentry* volatile)0xFFC06000;
	TSS* volatile KTSS = (TSS* volatile)0xFFC06068;
	TSS* volatile UTSS = (TSS* volatile)0xFFC060D0;
	IDTint* volatile IDT = (IDTint* volatile)0xFFC06200;
	
void kernel(void){
	initHeap(); //Setup kernel heap
	kdata = (volatile KernelData* volatile)(volatile uint32_t)k_KDATA; //Get kernel data
	clearVGA(); //Setup VGA

	//Display loading message
	vgaprint((volatile char* volatile)"Loading ...\n", 0x0F);

	//Setup keyboard
	KBDResetMods();
	
	//First we need to add 2 new page tables
	for (uint32_t* i = (uint32_t*)(0x400000 - 0x1000); i < (uint32_t*)0x400000; i++)
		*i = (((uint32_t)(i) + 0x1000 - 0x400000)/4 * 0x1000 + 0x400000) | 3;

	for (uint32_t* i = (uint32_t*)(0x400000 - 0x2000); i < (uint32_t*)(0x400000 - 0x1000); i++)
		*i = (((uint32_t)(i) + 0x2000 - 0x400000)/4 * 0x1000) | 3;

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

	//Get address info from ELF
	ElfPageList* pages;
	uint32_t sz;
	uint32_t entry;

	if (calcELF((uint8_t*)0x800, &pages, &sz) != 0){
		vgaprint((volatile char* volatile)"ERROR! The executable is not supported by this device\n.", 0x04);
		goto hang;
	}

	uint32_t defAppBP = sz - (sz % 0x400000);
	uint32_t* defAppPD = (uint32_t*)0x400000;
	uint32_t* defAppPT = (uint32_t*)(0x400000 + 0x1000);
	uint32_t* highPT = (uint32_t*)(0x400000 + 0x2000);

	uint32_t availP = 0;
	for (ElfPageList* i = pages; i->next != 0; i = i->next){
		defAppPT[(i->vaddr - defAppBP) / 0x1000] = (availP + 0x403000) | 7;
		availP += 0x1000;
	}

	defAppPT[1023] = (availP + 0x403000) | 7; //Stack page

	for (uint32_t i = 0; i < 1024; i++) 
		highPT[i] = (i * 0x1000) | 5; 

	defAppPD[defAppBP / 0x400000] = (0x400000 + 0x1000) | 7;
	defAppPD[1023] = (0x400000 + 0x2000) | 5;

	if (copyELF((uint8_t*)0x800, (uint8_t*)0x400000 + 0x3000, &entry) != 0){ //Copy the file after we know the paging layout
		vgaprint((volatile char* volatile)"ERROR! The executable is not supported by this device.\n", 0x04);
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
	KTSS->esp0 = 0xFFC06fdc;
	KTSS->ss0 = 16;
	KTSS->esp1 = 0x7c00;
	KTSS->ss1 = 16;
	KTSS->esp2 = 0x7c00;
	KTSS->ss2 = 16;
	KTSS->cr3 = k_CR3;
	KTSS->eip = (uint32_t)processManager;
	KTSS->eflags = 0; //0x200;
	KTSS->eax = 0;
	KTSS->ecx = 0;
	KTSS->edx = 0;
	KTSS->ebx = 0;
	KTSS->esp = 0xFFC06fdc;
	KTSS->ebp = 0xFFC07000;
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
	
	//User TSS
	UTSS->link = 48;
	UTSS->esp0 = 0xFFC06fdc;
	UTSS->ss0 = 16;
	UTSS->esp1 = 0x7c00;
	UTSS->ss1 = 16;
	UTSS->esp2 = 0x7c00;
	UTSS->ss2 = 16;
	UTSS->cr3 = (uint32_t)defAppPD;
	UTSS->eip = entry;
	UTSS->eflags = 0x200;
	UTSS->eax = 0;
	UTSS->ecx = 0;
	UTSS->edx = 0;
	UTSS->ebx = 0;
	UTSS->esp = defAppBP + 0x400000 - 4;
	UTSS->ebp = defAppBP + 0x400000;
	UTSS->esi = 0;
	UTSS->edi = 0;
	UTSS->es = 32 | 3;
	UTSS->cs = 24 | 3;
	UTSS->ss = 32 | 3;
	UTSS->ds = 32 | 3;
	UTSS->fs = 32 | 3;
	UTSS->gs = 32 | 3;
	UTSS->ldtd = 0;
	UTSS->IOPB = 0x68;
	
	//Setup system tables
	setSysTables();

	//Return to default application
	asm volatile ("xchg bx, bx" : : : "memory");
	asm volatile ("pushf \n pop ecx \n or ecx, 0x4200 \n \
			mov ax, 0x23 \n mov dx, ax \n mov es, ax \n mov fs, ax \n mov gs, ax \n \
		 	mov eax, esp \n push 0x23 \n push eax \n push ecx \n push 0x1b \n push %0 \n iret" : : "b"(entry): "memory");
	
	//hang
hang:
	while (1) asm volatile ("hlt" : : : "memory");
}

void processManager(uint32_t check, uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx, 
		uint32_t esi, uint32_t edi, uint32_t esp, uint32_t ebp){
	while (1) asm volatile ("hlt" : : : "memory"); //We haven't implemented userland interrupt handling yet
}
