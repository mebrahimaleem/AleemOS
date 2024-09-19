//xhci.h
//
//Declarations for the XHCI driver

typedef struct {
	uint32_t unk0;
	uint32_t unk1;
	uint32_t unk2;
	uint32_t cycle : 1;
	uint32_t unk3 : 9;
	uint32_t type : 6;
	uint32_t unk4 : 16;
} __attribute((packed)) TRB;

typedef struct {
	uint32_t ptrlo;
	uint32_t ptrhi;
	uint32_t cmplp : 24;
	uint32_t code : 8;
	uint32_t cycle : 1;
	uint32_t res0 : 9;
	uint32_t type : 6;
	uint32_t VFID : 8;
	uint32_t slot : 8;
} CmdCmplTRB;

typedef struct {
	uint32_t res0;
	uint32_t res1;
	uint32_t res2 : 22;
	uint32_t tar : 10;
	uint32_t cycle : 1;
	uint32_t ENT : 1;
	uint32_t res3 : 2;
	uint32_t CH : 1;
	uint32_t IOC : 1;
	uint32_t res4 : 4;
	uint32_t type : 6;
	uint32_t res5: 16;
} NoOpTRB;

typedef struct {
	uint32_t ptrlo;
	uint32_t ptrhi;
	uint32_t res0 : 22;
	uint32_t tar : 10;
	uint32_t cycle : 1;
	uint32_t TC : 1;
	uint32_t res1 : 2;
	uint32_t CH : 1;
	uint32_t IOC : 1;
	uint32_t res2 : 4;
	uint32_t type : 6;
	uint32_t res3 : 16;
} LinkTRB;

typedef struct {
	uint32_t addrlo;
	uint32_t addrhi;
	uint16_t sz;
	uint16_t res0;
	uint32_t res1;
} __attribute((packed)) ERSTE;

typedef struct XHCIHostData {
	uint8_t* volatile vMMIO;
	uint8_t CAPLENGTH;
	uint32_t RTSOFF;
	uint32_t DBOFF;
	uint8_t MaxSlots;
	uint32_t DCBAAP;
	ERSTE* volatile ERST;
	TRB* volatile commandRing;
	TRB* volatile commandRingPtr;
	TRB* volatile eventDequeue0;
	uint8_t CPCS;
	uint8_t ECCS0;
	struct XHCIHostData* next;
} XHCIHostData;

typedef struct TRBChain {
	TRB trb;
	struct TRBChain* next;
	struct TRBChain* p;
} TRBChain;

//TODO: make ERST struct

extern void initXHCIDriver(void);
extern uint64_t _xhciGetMMIO(PCIEntry ent);
extern uint32_t setupXHCIDevice(PCIEntry ent, uint32_t avAddr);
extern TRBChain* _xhciHandleEventRing(uint32_t i, XHCIHostData xhciD);
