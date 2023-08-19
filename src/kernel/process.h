//process.h
//
//This file has declarations for process management

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
	uint32_t IDN; //Identification number
	struct processState* next;
	uint32_t argc;
	uint32_t argv;
	uint32_t HS;
} processState;

/*
	Structure to hold setup information for a new process
*/
typedef struct processSetup {
	uint8_t res; //0 if process is ok, otherwise data is invalid
	processState state; //processState for the new process
	TSS utss; //TSS for the new process
	uint32_t codeB; //Base address for the start of the code

} processSetup;

/*
	Starts a process
	state: Pointer to process starting state
	toStart: 1 if process has never been started
*/
extern void startProcess(processState* state, uint8_t toStart);

/*
	Creates and starts a process
	state: The state of the process (latest state)
	cstate: The state for a process that has not been started (initial state)
*/
extern void createProcess(processState* state, processState* cstate);

/*
	Kills the process and resumes the parent process

	Returns 0 if no parent process exists
*/
extern uint32_t killProcess(void);

/*
	Creates paging structures for the process
	src: Pointer to executable

	Unlike the similar function in kernel/ELFParse.h, this function actually modifies the paging tables
*/
extern processSetup setupProcess(uint8_t* volatile src);

/*
	Resets current running processes memory and paging structure
*/
extern void resetProcessDivs(void);
