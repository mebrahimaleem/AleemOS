//xhci.c
//
//Implementations for the XHCI driver

#include <stdint.h>
#define DCBAAP_SIZE 0x1000 //In bytes
#define COMMAND_RING_SIZE 0x1000 //In bytes
#define ER_SIZE 0x1000 //In bytes
#define ERST_SIZE 0x1000 //In bytes
#define SCRBUFA_SIZE 0x1000 //In bytes

#define TRB_LINK_TYPE 6
#define TRB_NOOP_TYPE 8
#define TRB_CMDCMPL_TYPE 33

//Capability
#define CAPLENGTH_OFF 0x0
#define HCIVERSION_OFF 0x2
#define HCSPARAMS1_OFF 0x4
#define HCSPARAMS2_OFF 0x8
#define HCSPARAMS3_OFF 0xC
#define HCCPARAMS1_OFF 0x10
#define DBOFF_OFF 0x14
#define RTSOFF_OFF 0x18
#define HCCPARAMS2_OFF 0x1C

//Operational
#define USBCMD_OFF 0x0
#define USBSTS_OFF 0x4
#define PAGESIZE_OFF 0x8
#define DNCTRL_OFF 0x14
#define CRCR_OFF 0x18
#define DCBAAP_OFF 0x30
#define CONFIG_OFF 0x38
#define OP_PORT_OFF 0x400

//Runtime
#define MFINDEX_OFF 0x0
#define IR0_OFF 0x20

//IR
#define IMAN_OFF 0x0
#define ERSTS_OFF 0x8
#define ERDPA_OFF 0x18
#define ERSTBA_OFF 0x10

#define HANDLE_TRB_NONE 0

#define QMAX(a,b) (((uint32_t)a>(uint32_t)b?(uint32_t)a:(uint32_t)b) & 0xFFFFF000)

#define DRIVER_OK 0x0
#define DRIVER_FAIL 0x1

#include <memory.h>
#include <basicio.h>
#include <portio.h>
#include <paging.h>
#include <pci.h>
#include <xhci.h>

XHCIHostData* firstD;
XHCIHostData* lastD;
uint32_t devC;
uint32_t driverLastSts;

void initXHCIDriver() {
	firstD = lastD = 0;
	devC = 0;
	driverLastSts = DRIVER_OK;
	return;
}

uint64_t _xhciGetMMIO(PCIEntry ent) {
	return (uint64_t)(_pciReadDWord(ent.bus, ent.dev, ent.func, 0x10) & 0xFFFFFFF0) + 
		((uint64_t)(_pciReadDWord(ent.bus, ent.dev, ent.func, 0x14) & 0xFFFFFFFF) << 32);
}

uint32_t _xhciResetHost(PCIEntry ent, XHCIHostData xhciD, uint32_t avAddr) {
	driverLastSts = DRIVER_FAIL;

	// reset and init
	*(uint32_t* volatile)(xhciD.vMMIO + xhciD.CAPLENGTH + USBCMD_OFF) = 0x2; //stop and reset
	
	// wait for CNR
	while ((*(uint32_t* volatile)(xhciD.vMMIO + xhciD.CAPLENGTH + USBSTS_OFF) & 0x800) == 0x800) io_wait();

	//MaxSlotsEn
	*(uint32_t* volatile)(xhciD.vMMIO + xhciD.CAPLENGTH + CONFIG_OFF) = xhciD.maxSlots;

	//DCBAAP
	mapMemory(kernelPD, avAddr, avAddr, PAGING_KO);
	for (uint8_t* i = (uint8_t*)avAddr; i < (uint8_t*)(avAddr + 0x1000); i++) *i = 0;

	*(uint32_t* volatile)(xhciD.vMMIO + xhciD.CAPLENGTH + DCBAAP_OFF) = avAddr;
	*(uint32_t* volatile)(xhciD.vMMIO + xhciD.CAPLENGTH + DCBAAP_OFF + 0x4) = 0; //lo and hi
	xhciD.DCBAAP = avAddr;

	DCBAAE* volatile DCBAAE0 = (DCBAAE* volatile)avAddr;

	avAddr += DCBAAP_SIZE;

	//SCRBUFA
	mapMemory(kernelPD, avAddr, avAddr, PAGING_KO);
	for (uint8_t* i = (uint8_t*)avAddr; i < (uint8_t*)(avAddr + 0x1000); i++) *i = 0;

	DCBAAE0->addrlo = avAddr;
	DCBAAE0->addrhi = 0;
	avAddr += SCRBUFA_SIZE;

	uint32_t* scrBufE = (uint32_t*)avAddr;
	for (uint16_t i = 0; i < xhciD.maxScrBufs; i++) {
		*scrBufE = avAddr;
		for (uint32_t j = 0; j < xhciD.PAGESIZE; j++) {
			mapMemory(kernelPD, avAddr, avAddr, PAGING_KO);
			avAddr += 0x1000;
		}
		scrBufE += 2;
	}

	//Command ring
	mapMemory(kernelPD, avAddr, avAddr, PAGING_KO);
	*(uint32_t* volatile)(xhciD.vMMIO + xhciD.CAPLENGTH + CRCR_OFF) = avAddr | 0x1;
	*(uint32_t* volatile)(xhciD.vMMIO + xhciD.CAPLENGTH + CRCR_OFF + 0x4) = 0; //lo and hi
	xhciD.commandRing = xhciD.commandRingPtr = (TRB*)(avAddr);
	xhciD.CPCS = 1;
	xhciD.ECCS0 = 1;

	for (uint8_t* volatile i = (uint8_t*)avAddr; i < (uint8_t*)(avAddr + COMMAND_RING_SIZE); i++) *i = 0;

	LinkTRB* volatile linkTRB = (LinkTRB* volatile)(avAddr + COMMAND_RING_SIZE - 128); //dont need to write zeros bc the entire ring is filled with zeros
	linkTRB->ptrlo = avAddr;
	linkTRB->TC = 1;
	linkTRB->type = TRB_LINK_TYPE;

	avAddr += COMMAND_RING_SIZE;
	
	// disable all but the first interrupter TODO: use multiple interrupters (with MSI-X)
	mapMemory(kernelPD, avAddr, avAddr, PAGING_KO); // One page for ERST
	xhciD.ERST = (ERSTE* volatile)(avAddr);

	*(uint32_t* volatile)(xhciD.vMMIO + xhciD.RTSOFF + IR0_OFF + ERSTS_OFF + (32 * 0)) = 1; // 1 entry. TODO: Add more entries if needed
	avAddr += ERST_SIZE;

	mapMemory(kernelPD, avAddr, avAddr, PAGING_KO); //One page for the one ER entry
	xhciD.ERST->addrlo = avAddr;
	xhciD.ERST->addrhi = 0;
	xhciD.ERST->sz = 0x100; //TRB entries
	xhciD.eventDequeue0 = (TRB* volatile)avAddr;

	for (uint8_t* i = (uint8_t*)(avAddr); i < (uint8_t*)(avAddr + 0x1000); i++) *i = 0;

	// set ERDPA
	*(uint32_t* volatile)(xhciD.vMMIO + xhciD.RTSOFF + IR0_OFF + ERDPA_OFF + (32 * 0)) = avAddr;
	*(uint32_t* volatile)(xhciD.vMMIO + xhciD.RTSOFF + IR0_OFF + ERDPA_OFF + (32 * 0) + 4) = 0; //lo and hi

	// disable interrupts
	*(uint32_t* volatile)(xhciD.vMMIO + xhciD.RTSOFF + IR0_OFF + (32 * 0)) = 0;

	//set ERSTBA and start event ring
	*(uint32_t* volatile)(xhciD.vMMIO + xhciD.RTSOFF + IR0_OFF + IMAN_OFF + (32 * 0)) = 0x3; //write clear IP and set IE
	*(uint32_t* volatile)(xhciD.vMMIO + xhciD.RTSOFF + IR0_OFF + ERSTBA_OFF + (32 * 0)) = (uint32_t)xhciD.ERST;
	*(uint32_t* volatile)(xhciD.vMMIO + xhciD.RTSOFF + IR0_OFF + ERSTBA_OFF + (32 * 0) + 4) = 0; //lo and hi
	avAddr += ER_SIZE;

	for (uint32_t i = 1; i < 1024; i ++) {
		*(uint32_t* volatile)(xhciD.vMMIO + xhciD.RTSOFF + IR0_OFF + ERSTS_OFF + (32 * i)) = 0; // disable all other interupters
		*(uint32_t* volatile)(xhciD.vMMIO + xhciD.RTSOFF + IR0_OFF + (32 * i)) = 0;
	}
	
	// enable line interrupts
	uint8_t addr = findCapability(ent, 0x5); //MSI
	_pciWriteDWord(ent.bus, ent.dev, ent.func, addr, _pciReadDWord(ent.bus, ent.dev, ent.func, addr) & 0xFFFEFFFF); // disable
	
	addr = findCapability(ent, 0x11); //MSI-X
	_pciWriteDWord(ent.bus, ent.dev, ent.func, addr, _pciReadDWord(ent.bus, ent.dev, ent.func, addr) & 0x7FFFFFFF); // disable

	// start
	*(uint32_t* volatile)(xhciD.vMMIO + xhciD.CAPLENGTH + USBCMD_OFF) = 0x5; //run and interrupt enabled

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
		driverLastSts = DRIVER_OK;
	}

	return avAddr;
}

void _xhciISR(PCIEntry* ent, uint32_t opt0) {
	XHCIHostData xhciD = *(XHCIHostData*)ent->AVL;
	//TODO: switch to IO/APIC
	return;
}

uint32_t setupXHCIDevice(PCIEntry ent, uint32_t avAddr) {
	uint64_t MMIOaddr = _xhciGetMMIO(ent);
	if ((MMIOaddr >> 32) > 0) return avAddr; // If address cannot be stored in 32 bits, we will skip this device (since this is a 32-bit OS)
	
	uint32_t cap = (uint32_t)(MMIOaddr & 0xFFFFF000);

	XHCIHostData xhciD;
	mapMemory(kernelPD, avAddr, cap, PAGING_KO);
	xhciD.vMMIO = (uint8_t* volatile)avAddr;
	xhciD.CAPLENGTH = *xhciD.vMMIO;
	xhciD.DBOFF = *(uint32_t* volatile)(xhciD.vMMIO + DBOFF_OFF);
	xhciD.maxSlots = *(uint32_t* volatile)(xhciD.vMMIO + HCSPARAMS1_OFF) & 0xFF;
	xhciD.RTSOFF = *(uint32_t* volatile)(xhciD.vMMIO + RTSOFF_OFF);
	xhciD.maxScrBufs = ((*(uint32_t* volatile)(xhciD.vMMIO + HCSPARAMS2_OFF) >> 16) & 0x3E0) + ((*(uint32_t* volatile)(xhciD.vMMIO + HCSPARAMS2_OFF) >> 27) & 0x1F);
	xhciD.PAGESIZE = *(uint32_t* volatile)(xhciD.vMMIO + xhciD.CAPLENGTH + PAGESIZE_OFF);

	ent.handler = _xhciISR;
	ent.AVL = (void*)(&xhciD);

	mapMemory(kernelPD, avAddr + xhciD.CAPLENGTH, cap + xhciD.CAPLENGTH, PAGING_KO); //Operational
	mapMemory(kernelPD, avAddr + xhciD.CAPLENGTH + 0x1000, cap + xhciD.CAPLENGTH + 0x1000, PAGING_KO);

	mapMemory(kernelPD, avAddr + xhciD.RTSOFF, cap + xhciD.RTSOFF, PAGING_KO); // Runtime
	mapMemory(kernelPD, avAddr + xhciD.RTSOFF + 0x1000, cap + xhciD.RTSOFF + 0x1000, PAGING_KO);
	mapMemory(kernelPD, avAddr + xhciD.RTSOFF + 0x2000, cap + xhciD.RTSOFF + 0x2000, PAGING_KO);
	mapMemory(kernelPD, avAddr + xhciD.RTSOFF + 0x3000, cap + xhciD.RTSOFF + 0x3000, PAGING_KO);
	mapMemory(kernelPD, avAddr + xhciD.RTSOFF + 0x4000, cap + xhciD.RTSOFF + 0x4000, PAGING_KO);
	mapMemory(kernelPD, avAddr + xhciD.RTSOFF + 0x5000, cap + xhciD.RTSOFF + 0x5000, PAGING_KO);
	mapMemory(kernelPD, avAddr + xhciD.RTSOFF + 0x6000, cap + xhciD.RTSOFF + 0x6000, PAGING_KO);
	mapMemory(kernelPD, avAddr + xhciD.RTSOFF + 0x7000, cap + xhciD.RTSOFF + 0x7000, PAGING_KO);
	mapMemory(kernelPD, avAddr + xhciD.RTSOFF + 0x8000, cap + xhciD.RTSOFF + 0x8000, PAGING_KO);

	mapMemory(kernelPD, avAddr + xhciD.DBOFF, cap + xhciD.DBOFF, PAGING_KO); // Doorbell

	avAddr += QMAX(QMAX(xhciD.DBOFF, xhciD.RTSOFF + 0x8000), xhciD.CAPLENGTH + 0x1000) + 0x1000;

	avAddr = _xhciResetHost(ent, xhciD, avAddr);
	if (driverLastSts == DRIVER_OK) {
		vgaprint("xhC is initialized\n", 0x0A);
	}
	else {
		vgaprint("ERROR: Failed to init xhC\n", 0x0E);
	}

	return avAddr;
}

void enumerateAllXHCI() {
	for (XHCIHostData* i = firstD; i != 0; i = i->next) _xhciEnumerate(*i);
}

void _xhciEnumerate(XHCIHostData xhciD) {
	// reset root hub ports
	for (uint32_t i = 0; i < xhciD.maxSlots; i++) {
		// TODO: 4.3.1 and ennumerate.
	}
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
					*(uint32_t* volatile)(xhciD.vMMIO + xhciD.RTSOFF + IR0_OFF + ERSTBA_OFF + (32 * i)) = (uint32_t)xhciD.eventDequeue0;
					*(uint32_t* volatile)(xhciD.vMMIO + xhciD.RTSOFF + IR0_OFF + ERSTBA_OFF + (32 * i) + 4) = 0; //lo and hi
				}
				// TODO: implement limit check
				break;
			default:
				break;
		}
	}
	return chain;
}
