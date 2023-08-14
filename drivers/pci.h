//pci.h
//
// Declarations for PCI control 

typedef enum {
	UNKOWN,
	USB_UHCI,
	USB_OHCI,
	USB_EHCI,
	USB_XHCI,
	USB_NOIF, //No Programming interface
	USB_NOHC //No Host controller
} PCIType;

typedef struct PCIEntry {
	uint8_t bus;
	uint8_t dev;
	PCIType type;
	struct PCIEntry* next;
} PCIEntry;

extern uint32_t _pciReadDWord(uint8_t bus, uint8_t dev, uint8_t func, uint8_t addr);
extern void _pciWriteDWord(uint8_t bus, uint8_t dev, uint8_t func, uint8_t addr, uint32_t dat);
extern PCIEntry* getPCIDevices(void);
