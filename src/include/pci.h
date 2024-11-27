//pci.h
//
// Declarations for PCI control 
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

typedef enum {
	UNKOWN,		//													Base Sub	Interface
	IDE_CTR, // unkown IDE controller			0x01 0x01
	ETHN_CTR, // Ethernet Controller			0x02 0x00 0x00
	VGA_CTR, //VGA Controller							0x03 0x00 0x00
	HOST_BRIDGE, // PCI Host Bridge				0x06 0x00 0x00
	OTHER_BRIDGE, //Other Bridge Device		0x06 0x80 0x00
	ISA_BRIDGE, // ISA Bridge							0x06 0x01 0x00
	USB_UHCI, // UHCI USB									0x0c 0x03 0x00
	USB_OHCI, // OHCI USB									0x0c 0x03 0x10
	USB_EHCI, // EHCI USB									0x0c 0x03 0x20
	USB_XHCI, // XHCI USB									0x0c 0x03 0x30
	USB_NOIF, //USB with no interface			0x0c 0x03 0x80
	USB_NOHC //USB with no host						0x0c 0x03 0xfe
} PCIType;

struct PCIEntry;

typedef void (*pciHandler)(struct PCIEntry*, uint32_t);

typedef struct PCIEntry {
	uint8_t bus;
	uint8_t dev;
	uint8_t func;
	uint8_t irqLine;
	PCIType type;
	pciHandler handler;
	void* AVL;
	struct PCIEntry* next;
} PCIEntry;

extern uint32_t _pciReadDWord(uint8_t bus, uint8_t dev, uint8_t func, uint8_t addr);
extern void _pciWriteDWord(uint8_t bus, uint8_t dev, uint8_t func, uint8_t addr, uint32_t dat);
extern PCIEntry* getPCIDevices(void);
extern void ISR2AB_handler(uint32_t opt0);
extern uint8_t findCapability(PCIEntry ent, uint8_t id);
