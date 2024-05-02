//xhci.h
//
//Declarations for the XHCI driver

typedef struct XHCIHostData{
	uint8_t* vMMIO;
	uint8_t CAPLENGTH;
	struct XHCIHostData* next;
} XHCIHostData;

extern void initXHCIDriver(void);
extern uint64_t _xhciGetMMIO(PCIEntry ent);
extern uint32_t setupXHCIDevice(PCIEntry ent, uint32_t avAddr);
