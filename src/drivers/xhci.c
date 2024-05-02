//xhci.c
//
//Implementations for the XHCI driver

#include <stdint.h>
#define MMIO_SIZE 0x1000 //In bytes

#include <memory.h>
#include <basicio.h>
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

	XHCIHostData xhciD;
	mapMemory(kernelPD, avAddr, ((uint32_t)MMIOaddr & 0xFFFFF000), 3); // TODO: add more pages as needed
	xhciD.vMMIO = (uint8_t*)avAddr;
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

	return avAddr + MMIO_SIZE;
}
