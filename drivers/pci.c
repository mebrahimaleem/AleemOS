//pci.c
//
// Implementation for PCI control

#include <stdint.h>

#include "../kernel/portio.h"
#include "../kernel/basicio.h"
#include "../kernel/memory.h"
#include "pci.h"
#include "../kernel/kernel.h"

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
		for (uint8_t dev = 0; dev < 32; dev++)
			for (uint16_t func = 0; func < 8; func++) {	
				if ((_pciReadDWord((uint8_t)bus, dev, (uint8_t)func, 0) & 0xFFFF) == 0xFFFF) continue;
				if (devs == 0)
					devs = last = (PCIEntry*)malloc(sizeof(PCIEntry));

				else {
					last->next = (PCIEntry*)malloc(sizeof(PCIEntry));
					last = last->next;
				}

				last->bus = (uint8_t)bus;
				last->dev = dev;
				last->irqLine = _pciReadDWord((uint8_t)bus, dev, (uint8_t)func, 0x3C) & 0xFF; //Always works regarldess of header type
				last->func = (uint8_t)func; //TODO: Later implement for adding other functions to the PCI device list
				last->next = 0;
		
				switch (_pciReadDWord((uint8_t)bus, dev, (uint8_t)func, 8) >> 8) {
					case 0x0C0300:
						last->type = USB_UHCI;
						break;
					case 0x0C0310:
						last->type = USB_OHCI;
						break;
					case 0x0C0320:
						last->type = USB_EHCI;
						break;
					case 0x0C0330:
						last->type = USB_XHCI;
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
				
				if (func == 0) {
					vgaprint((volatile char* volatile)"Found PCI Device. Bus: 0x", 0x0A);
					vgaprintint((uint8_t)bus, 16, 0x0A);
					vgaprint((volatile char* volatile)" Device: 0x", 0x0A);
					vgaprintint(dev, 16, 0x0A);
					vgaprint((volatile char* volatile)" Type: ", 0x0A);
					vgaprintint(last->type, 10, 0x0A);

					if (last->irqLine != 0) {
						vgaprint((volatile char* volatile)" IRQ: 0x", 0x0A);
						vgaprintint((last->irqLine & 0xFF) + 0x20, 16, 0x0A);
					}

					vgaprint((volatile char* volatile)"\n", 0x0A);
			}
				if (((_pciReadDWord((uint8_t)bus, dev, 0, 0xC) >> 16) & 0x80) != 0x80) break;
			}

	return devs;
}

#pragma GCC push_options
#pragma GCC optimize("O0")
void ISR2AB_handler(uint32_t opt0) {
	for (PCIEntry* i = pciEntries; i != 0; i = i->next) {
		if (i->irqLine == (opt0 & 0xFF) && (_pciReadDWord(i->bus, i->dev, i->func, 0x1) & 0x80000) == 0x80000) {
			//TODO: handle device
			return;
		}
	}
	return;
}
#pragma GCC pop_options
