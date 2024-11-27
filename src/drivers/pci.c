//pci.c
//
// Implementation for PCI control
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

#include <stdint.h>

#include <portio.h>
#include <basicio.h>
#include <memory.h>
#include <pci.h>
#include <kernel.h>

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
				last->handler = 0;
		
				uint32_t classCode = _pciReadDWord((uint8_t)bus, dev, (uint8_t)func, 8) >> 8;
				switch (classCode) {
					// IDE controllers will be handled in default
					// TODO: implement better way to handle IDE controller
					case 0x020000:
						last->type = ETHN_CTR;
						break;
					case 0x030000:
						last->type = VGA_CTR;
						break;
					case 0x060000:
						last->type = HOST_BRIDGE;
						break;
					case 0x068000:
						last->type = OTHER_BRIDGE;
						break;
					case 0x060100:
						last->type = ISA_BRIDGE;
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
						if ((classCode & 0xFFFF00) == 0x010100) // IDE Controller, unkown interface
							last->type = IDE_CTR;
						else
							last->type = UNKOWN;
						break;
				}
				
				if (func == 0 || last->type != UNKOWN) {
					vgaprint("Found PCI Device. Bus: 0x", 0x0A);
					vgaprintint((uint8_t)bus, 16, 0x0A);
					vgaprint(" Device: 0x", 0x0A);
					vgaprintint(dev, 16, 0x0A);
					vgaprint(" Func: 0x", 0x0A);
					vgaprintint(func, 16, 0x0A);
					vgaprint(" Type: ", 0x0A);
					vgaprintint(last->type, 16, 0x0A);

					if (last->irqLine != 0) {
						vgaprint(" IRQ: 0x", 0x0A);
						vgaprintint((last->irqLine & 0xFF) + 0x20, 16, 0x0A);
					}

					vgaprint("\n", 0x0A);
			}
				if (((_pciReadDWord((uint8_t)bus, dev, 0, 0xC) >> 16) & 0x80) != 0x80) break;
			}

	return devs;
}

uint8_t findCapability(PCIEntry ent, uint8_t id) {
	if ((_pciReadDWord(ent.bus, ent.dev, ent.func, 0x4) & 0xFFFF0000) != 0x100000) return 0; // does not support capabilities
	uint8_t off = _pciReadDWord(ent.bus, ent.dev, ent.func, 0x34) & 0xFC;
	while (1) { //TODO: check for end of capabilities
		if ((_pciReadDWord(ent.bus, ent.dev, ent.func, off) & 0xFF) == id) return off;
		off = (_pciReadDWord(ent.bus, ent.dev, ent.func, off) & 0xFC00) >> 8;
	}
}

#pragma GCC push_options
#pragma GCC optimize("O0")
void ISR2AB_handler(uint32_t opt0) {
	for (PCIEntry* i = pciEntries; i != 0; i = i->next) {
		if (i->irqLine == (opt0 & 0xFF) && (_pciReadDWord(i->bus, i->dev, i->func, 0x1) & 0x80000) == 0x80000) {
			if (i->handler != 0) i->handler(i, opt0);
			return;
		}
	}
	return;
}
#pragma GCC pop_options
