//pci.h
//
// Declarations for PCI control 

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

typedef struct PCIEntry {
	uint8_t bus;
	uint8_t dev;
	uint8_t func;
	uint8_t irqLine;
	PCIType type;
	struct PCIEntry* next;
} PCIEntry;

extern uint32_t _pciReadDWord(uint8_t bus, uint8_t dev, uint8_t func, uint8_t addr);
extern void _pciWriteDWord(uint8_t bus, uint8_t dev, uint8_t func, uint8_t addr, uint32_t dat);
extern PCIEntry* getPCIDevices(void);
extern void ISR2AB_handler(uint32_t opt0);
