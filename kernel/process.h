//process.h
//
//This file has declarations for process management

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
	uint32_t IDN; //Identification number
	struct processState* next;
	uint32_t argc;
	uint32_t argv;
	uint32_t HS;
} processState;

typedef struct processSetup {
	uint8_t res; //0 if process is ok, otherwise data is invalid
	processState state; //processState for the new process
	TSS utss; //TSS for the new process
	uint32_t codeB; //Base address for the start of the code

} processSetup;

//Starts a process
extern void startProcess(processState* state, uint8_t toStart);

//Creates and starts a new process with the processState 'state'
extern void createProcess(processState* state, processState* cstate);

//Kill the current process and provides the next process to run, returns 0 if no processes remain
extern uint32_t killProcess(void);

//Creates paging structures for a new process
extern processSetup setupProcess(uint8_t* volatile src);

//Resets process paging information - discards all process memory
extern void resetProcessDivs(void);
