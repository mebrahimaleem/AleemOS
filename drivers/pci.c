//pci.c
//
// Implementation for PCI control

#include <stdint.h>

#include "../kernel/portio.h"
#include "../kernel/memory.h"
#include "pci.h"

#define CONFIG_ADDRESS 0xCF8
#define CONFIG_DATA 0xCFC

uint32_t _pciReadDWord(uint8_t bus, uint8_t dev, uint8_t func, uint8_t addr) {
	uint32_t r = (addr & 0xFC) | (func << 8) | (dev << 11) | (bus << 16) | (0x80000000);
	outd(CONFIG_ADDRESS, r);
	io_wait();
	return ind(CONFIG_DATA);
}

void _pciWriteDWord(uint8_t bus, uint8_t dev, uint8_t func, uint8_t addr, uint32_t dat) {
	uint32_t r = (addr & 0xFC) | (func << 8) | (dev << 11) | (bus << 16) | (0x80000000);
	outd(CONFIG_ADDRESS, r);
	io_wait();
	outd(CONFIG_DATA, dat);
	return;
}

PCIEntry* getPCIDevices() {
	PCIEntry* devs = 0;
	PCIEntry* last;
	for (uint16_t bus = 0; bus < 256; bus++)
		for (uint8_t dev = 0; dev < 32; dev++) {
			if ((_pciReadDWord((uint8_t)bus, dev, 0, 0) & 0xFFFF) == 0xFFFF) continue;
			if (devs == 0)
				devs = last = (PCIEntry*)malloc(sizeof(PCIEntry));

			else {
				last->next = (PCIEntry*)malloc(sizeof(PCIEntry));
				last = last->next;
			}

			last->bus = (uint8_t)bus;
			last->dev = dev;
			last->next = 0;
	
			switch (_pciReadDWord((uint8_t)bus, dev, 0, 8) >> 8) {
				case 0x0C0330:
					last->type = USB_XHCI;
					break;
				case 0x0C0300:
					last->type = USB_UHCI;
					break;
				case 0x0C0310:
					last->type = USB_OHCI;
					break;
				case 0x0C0320:
					last->type = USB_EHCI;
					break;
				case 0x0C0380:
					last->type = USB_NOIF;
					break;
				case 0x0C03FE:
					last->type = USB_NOHC;
					break;
				default:
					last->type = UNKOWN;
					break;
			}
		}

	return devs;
}
