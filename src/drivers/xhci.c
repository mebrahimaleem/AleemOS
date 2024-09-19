//xhci.c
//
//Implementations for the XHCI driver

#include <stdint.h>
#define DCBAAP_SIZE 0x1000 //In bytes
#define COMMAND_RING_SIZE 0x1000 //In bytes
#define ER_SIZE 0x1000 //In bytes
#define ERST_SIZE 0x1000 //In bytes

#define TRB_LINK_TYPE 6
#define TRB_NOOP_TYPE 8
#define TRB_CMDCMPL_TYPE 33

#define HANDLE_TRB_NONE 0

#define QMAX(a,b) (((uint32_t)a>(uint32_t)b?(uint32_t)a:(uint32_t)b) & 0xFFFFF000)

#include <memory.h>
#include <basicio.h>
#include <portio.h>
#include <paging.h>
#include <pci.h>
#include <xhci.h>

XHCIHostData* firstD;
XHCIHostData* lastD;
uint32_t devC;

void initXHCIDriver() {
	firstD = lastD = 0;
	devC = 0;
	return;
}

uint64_t _xhciGetMMIO(PCIEntry ent) {
	return (uint64_t)(_pciReadDWord(ent.bus, ent.dev, ent.func, 0x10) & 0xFFFFFFF0) + 
		((uint64_t)(_pciReadDWord(ent.bus, ent.dev, ent.func, 0x14) & 0xFFFFFFFF) << 32);
}

uint32_t setupXHCIDevice(PCIEntry ent, uint32_t avAddr) {
	uint64_t MMIOaddr = _xhciGetMMIO(ent);
	if ((MMIOaddr >> 32) > 0) return avAddr; // If address cannot be stored in 32 bits, we will skip this device (since this is a 32-bit OS)
	
	uint32_t cap = (uint32_t)(MMIOaddr & 0xFFFFF000);

	XHCIHostData xhciD;
	mapMemory(kernelPD, avAddr, cap, 3);
	xhciD.vMMIO = (uint8_t* volatile)avAddr;
	xhciD.CAPLENGTH = *xhciD.vMMIO;
	xhciD.DBOFF = *(uint32_t* volatile)(xhciD.vMMIO + 0x14);
	xhciD.MaxSlots = *(uint32_t* volatile)(xhciD.vMMIO + 0x4) & 0xFF;
	xhciD.RTSOFF = *(uint32_t* volatile)(xhciD.vMMIO + 0x18);

	mapMemory(kernelPD, avAddr + xhciD.CAPLENGTH, cap + xhciD.CAPLENGTH, 3); //Operational
	mapMemory(kernelPD, avAddr + xhciD.CAPLENGTH + 0x1000, cap + xhciD.CAPLENGTH + 0x1000, 3);

	mapMemory(kernelPD, avAddr + xhciD.RTSOFF, cap + xhciD.RTSOFF, 3); // Runtime
	mapMemory(kernelPD, avAddr + xhciD.RTSOFF + 0x1000, cap + xhciD.RTSOFF + 0x1000, 3);
	mapMemory(kernelPD, avAddr + xhciD.RTSOFF + 0x2000, cap + xhciD.RTSOFF + 0x2000, 3);
	mapMemory(kernelPD, avAddr + xhciD.RTSOFF + 0x3000, cap + xhciD.RTSOFF + 0x3000, 3);
	mapMemory(kernelPD, avAddr + xhciD.RTSOFF + 0x4000, cap + xhciD.RTSOFF + 0x4000, 3);
	mapMemory(kernelPD, avAddr + xhciD.RTSOFF + 0x5000, cap + xhciD.RTSOFF + 0x5000, 3);
	mapMemory(kernelPD, avAddr + xhciD.RTSOFF + 0x6000, cap + xhciD.RTSOFF + 0x6000, 3);
	mapMemory(kernelPD, avAddr + xhciD.RTSOFF + 0x7000, cap + xhciD.RTSOFF + 0x7000, 3);
	mapMemory(kernelPD, avAddr + xhciD.RTSOFF + 0x8000, cap + xhciD.RTSOFF + 0x8000, 3);

	mapMemory(kernelPD, avAddr + xhciD.DBOFF, cap + xhciD.DBOFF, 3); // Doorbell

	avAddr += QMAX(QMAX(xhciD.DBOFF, xhciD.RTSOFF + 0x8000), xhciD.CAPLENGTH + 0x1000) + 0x1000;

	// reset and init
	*(uint32_t* volatile)(xhciD.vMMIO + xhciD.CAPLENGTH + 0) = 0x2; //stop and reset
	
	// wait for CNR
	while ((*(uint32_t* volatile)(xhciD.vMMIO + xhciD.CAPLENGTH + 0x4) & 0x800) == 0x800) io_wait();

	//MaxSlotsEn
	*(uint32_t* volatile)(xhciD.vMMIO + xhciD.CAPLENGTH + 0x38) = xhciD.MaxSlots;

	//DCBAAP
	mapMemory(kernelPD, avAddr, avAddr, 3);
	*(uint32_t* volatile)(xhciD.vMMIO + xhciD.CAPLENGTH + 0x30) = avAddr;
	*(uint32_t* volatile)(xhciD.vMMIO + xhciD.CAPLENGTH + 0x34) = 0;
	xhciD.DCBAAP = avAddr;

	avAddr += DCBAAP_SIZE;

	//Command ring
	mapMemory(kernelPD, avAddr, avAddr, 3);
	*(uint32_t* volatile)(xhciD.vMMIO + xhciD.CAPLENGTH + 0x18) = avAddr | 0x1;
	*(uint32_t* volatile)(xhciD.vMMIO + xhciD.CAPLENGTH + 0x1C) = 0;
	xhciD.commandRing = xhciD.commandRingPtr = (TRB*)(avAddr);
	xhciD.CPCS = 1;
	xhciD.ECCS0 = 1;

	for (uint8_t* volatile i = (uint8_t*)avAddr; i < (uint8_t*)(avAddr + COMMAND_RING_SIZE); i++) *i = 0;

	LinkTRB* volatile linkTRB = (LinkTRB* volatile)(avAddr + COMMAND_RING_SIZE - 128); //dont need to write zeros bc the entire ring is filled with zeros
	linkTRB->ptrlo = avAddr;
	linkTRB->TC = 1;
	linkTRB->type = TRB_LINK_TYPE;

	avAddr += COMMAND_RING_SIZE;
	
	// disable all but the first interrupter TODO: use multiple interrupters
	mapMemory(kernelPD, avAddr, avAddr, 3); // One page for ERST
	xhciD.ERST = (ERSTE* volatile)(avAddr);

	*(uint32_t* volatile)(xhciD.vMMIO + xhciD.RTSOFF + 0x28 + (32 * 0)) = 1; // 1 entry. TODO: Add more entries if needed
	avAddr += ERST_SIZE;

	mapMemory(kernelPD, avAddr, avAddr, 3); //One page for the one ER entry
	xhciD.ERST->addrlo = avAddr;
	xhciD.ERST->addrhi = 0;
	xhciD.ERST->sz = 0x100; //TRB entries
	xhciD.eventDequeue0 = (TRB* volatile)avAddr;

	for (uint8_t* i = (uint8_t*)(avAddr); i < (uint8_t*)(avAddr + 0x1000); i++) *i = 0;

	// set ERDP
	*(uint32_t* volatile)(xhciD.vMMIO + xhciD.RTSOFF + 0x38 + (32 * 0)) = avAddr;
	*(uint32_t* volatile)(xhciD.vMMIO + xhciD.RTSOFF + 0x38 + (32 * 0) + 4) = 0;

	// disable interrupts
	*(uint32_t* volatile)(xhciD.vMMIO + xhciD.RTSOFF + 0x20 + (32 * 0)) = 0;

	//set ERSTBA and start event ring
	*(uint32_t* volatile)(xhciD.vMMIO + xhciD.RTSOFF + 0x30 + (32 * 0)) = (uint32_t)xhciD.ERST;
	*(uint32_t* volatile)(xhciD.vMMIO + xhciD.RTSOFF + 0x30 + (32 * 0) + 4) = 0;
	avAddr += ER_SIZE;

	for (uint32_t i = 1; i < 1024; i ++) {
		*(uint32_t* volatile)(xhciD.vMMIO + xhciD.RTSOFF + 0x28 + (32 * i)) = 0; // disable all other interupters
		*(uint32_t* volatile)(xhciD.vMMIO + xhciD.RTSOFF + 0x20 + (32 * i)) = 0;
	}
	
	// disable interrupts TODO: implement interrupts
	*(uint32_t* volatile)(xhciD.vMMIO + xhciD.CAPLENGTH + 0x0) = 0;

	// start
	*(uint32_t* volatile)(xhciD.vMMIO + xhciD.CAPLENGTH + 0x00) = 0x1;

	// verify command ring is working by sending no op
	uint32_t checkNoOp = (uint32_t)xhciD.commandRingPtr;
	NoOpTRB* volatile noop = (NoOpTRB* volatile)xhciD.commandRingPtr;
	noop->res0 = 0;
	noop->res1 = 0;
	noop->res2 = 0;
	noop->res3 = 0;
	noop->res4 = 0;
	noop->res5 = 0;
	noop->tar = 0;
	noop->cycle = 1;
	noop->ENT = 0;
	noop->CH = 0;
	noop->IOC = 0;
	noop->type = TRB_NOOP_TYPE;
	xhciD.commandRingPtr++;

	*(uint32_t* volatile)(xhciD.vMMIO + xhciD.DBOFF + 0x00) = 0; // ring doorbell

	//process event ring for interrupter 0
	uint8_t suc = 0;
	TRBChain* chain;
	for (chain = _xhciHandleEventRing(0, xhciD); chain->next != 0; chain = chain->next) {
		if (chain->p != 0) free(chain->p);
		if (chain->trb.type == TRB_CMDCMPL_TYPE && ((CmdCmplTRB*)(&(chain->trb)))->ptrlo == checkNoOp) suc = 1;
	}
	free(chain);
	
	if (suc) {
		// Add to devices list
		if (firstD == 0)
			firstD = lastD = (XHCIHostData*)malloc(sizeof(XHCIHostData));
		else {
			lastD->next = (XHCIHostData*)malloc(sizeof(XHCIHostData));
			lastD = lastD->next;
		}
		*lastD = xhciD;
		lastD->next = 0;
		vgaprint("xhC is initialized\n", 0x0A);
	}
	else {
		vgaprint("ERROR: Failed to init xhC\n", 0x0E);
	}

	//TODO: enumerate attached devices

	return avAddr;
}

TRBChain* _xhciHandleEventRing(uint32_t i, XHCIHostData xhciD) {
	TRBChain* chain = (TRBChain*)malloc(sizeof(TRBChain));
	TRBChain* n = chain;
	n->p = 0;
	uint8_t c = 1;
	while (c) {
		switch (i) {
			case 0:
				n->trb = *(xhciD.eventDequeue0);
				switch (n->trb.type) {
					case TRB_CMDCMPL_TYPE:
						break; //TODO: handle/log
					default:
						vgaprint("WARNING: Unexpected TRB\n", 0x0E);
						break;
				}
				n->next = (TRBChain*)malloc(sizeof(TRBChain));
				n->next->p = n;
				n = n->next;

				xhciD.eventDequeue0++;
				if (xhciD.eventDequeue0->cycle != xhciD.ECCS0) {
					c = 0;
					*(uint32_t* volatile)(xhciD.vMMIO + xhciD.RTSOFF + 0x30 + (32 * i)) = (uint32_t)xhciD.eventDequeue0;
					*(uint32_t* volatile)(xhciD.vMMIO + xhciD.RTSOFF + 0x30 + (32 * i) + 4) = 0;
				}
				// TODO: implement limit check
				break;
			default:
				break;
		}
	}
	return chain;
}
