//process.h
//
//This file has declarations for process management
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

/*
	Holds information on a process' state
*/
typedef struct processState {
	uint32_t eax;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
	uint32_t esi;
	uint32_t edi;
	uint32_t esp;
	uint32_t ebp;
	uint32_t eip;
	uint32_t eflags;
	uint32_t cr3;
	uint32_t PID;
	uint32_t argc;
	uint32_t HS;
	uint8_t priority; //bits 0-6: priority; bit 7: toStart
	uint32_t kHeapVaddr; // kernel virtual address to process' heap
	struct processState* next;
} __attribute((packed)) processState;

/*
	Structure to hold setup information for a new process
*/
typedef struct processSetup {
	uint8_t res; //0 if process is ok, otherwise data is invalid
	processState state; //processState for the new process

} processSetup;

/*
	Starts a process
	state: Pointer to process starting state
*/
extern void startProcess(processState* state);

/*
	Creates paging structures for the process
	src: Pointer to executable

	Unlike the similar function in kernel/ELFParse.h, this function actually modifies the paging tables
*/
extern processSetup setupProcess(uint8_t* src, uint8_t priority, uint32_t argc, uint8_t** argv);

/*
	Resets current running processes memory and paging structure
*/
extern void resetProcessDivs(void);

extern uint8_t pageDivs[128];
