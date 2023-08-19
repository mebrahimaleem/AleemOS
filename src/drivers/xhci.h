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
extern uint16_t setupXHCIDevice(PCIEntry ent, uint16_t avPT);
