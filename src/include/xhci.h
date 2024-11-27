//xhci.h
//
//Declarations for the XHCI driver
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
} __attribute((packed)) CmdCmplTRB;

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
} __attribute((packed)) NoOpTRB;

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
} __attribute((packed)) LinkTRB;

typedef struct {
	uint32_t addrlo;
	uint32_t addrhi;
	uint16_t sz;
	uint16_t res0;
	uint32_t res1;
} __attribute((packed)) ERSTE;

typedef struct {
	uint32_t addrlo;
	uint32_t addrhi;
} __attribute((packed)) DCBAAE;

typedef struct XHCIHostData {
	uint8_t* volatile vMMIO;
	uint8_t CAPLENGTH;
	uint32_t RTSOFF;
	uint32_t DBOFF;
	uint8_t maxSlots;
	uint32_t DCBAAP;
	ERSTE* volatile ERST;
	TRB* volatile commandRing;
	TRB* volatile commandRingPtr;
	TRB* volatile eventDequeue0;
	uint8_t CPCS;
	uint8_t ECCS0;
	uint16_t maxScrBufs;
	uint32_t PAGESIZE;
	struct XHCIHostData* next;
} XHCIHostData;

typedef struct TRBChain {
	TRB trb;
	struct TRBChain* next;
	struct TRBChain* p;
} TRBChain;

extern void initXHCIDriver(void);
extern uint64_t _xhciGetMMIO(PCIEntry ent);
extern uint32_t setupXHCIDevice(PCIEntry ent, uint32_t avAddr);
extern TRBChain* _xhciHandleEventRing(uint32_t i, XHCIHostData xhciD);
extern void enumerateAllXHCI(void);
extern void _xhciEnumerate(XHCIHostData xhciD);
extern uint32_t _xhciResetHost(PCIEntry ent, XHCIHostData xhciD, uint32_t avAddr);
extern void _xhciISR(PCIEntry* ent, uint32_t opt0);
