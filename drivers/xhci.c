//xhci.c
//
//Implementations for the XHCI driver

#include <stdint.h>
#define MMIO_SIZE 1 //In pages

#include "../kernel/memory.h"
#include "../kernel/basicio.h"
#include "pci.h"
#include "xhci.h"

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

uint16_t setupXHCIDevice(PCIEntry ent, uint16_t avPT) {
	uint32_t cmstat = _pciReadDWord(ent.bus, ent.dev, ent.func, 0x4);
	_pciWriteDWord(ent.bus, ent.dev, ent.func, 0x4, cmstat & 0xFFFFFFFC); //Stop all unwanted PCI device behavior
	_pciWriteDWord(ent.bus, ent.dev, ent.func, 0x4, cmstat); //Restore PCI command register state

	uint64_t MMIOaddr = _xhciGetMMIO(ent);
	if ((MMIOaddr >> 32) > 0) return avPT; // If address cannot be stored in 32 bits, we will skip this device (since this is a 32-bit OS)

	XHCIHostData xhciD;
	*(volatile uint32_t* volatile)(0x400000 - 0x3000 + avPT * 4) = ((uint32_t)MMIOaddr & 0xFFFFF000)  | 3; //TODO: add more pages as needed
	xhciD.vMMIO = (uint8_t*)(1 + 0xFFFFFFFF - 0x800000 + avPT * 0x1000);
	xhciD.CAPLENGTH = *xhciD.vMMIO;

	// Add to devices list
	if (firstD == 0)
		firstD = lastD = (XHCIHostData*)malloc(sizeof(XHCIHostData));
	else {
		lastD->next = (XHCIHostData*)malloc(sizeof(XHCIHostData));
		lastD = lastD->next;
	}

	*lastD = xhciD;
	lastD->next = 0;

	return avPT + MMIO_SIZE;
}
