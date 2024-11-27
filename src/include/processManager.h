
/*
	Called to manage intel reserved IRQs
	check: The interrupt code (see implementation in kernel/kentry.asm)
*/
extern void processManager(uint32_t check);

/*
	Called to bridge userland to kernel
	call: System call number (see implementation)
	params: Pointer to parameter in userland (or the parameter itself)
*/
extern uint32_t sysCall(uint32_t call, uint32_t params);

/*
	Converts the memory of a processes heap to a kernel virtual address
	kHeapVaddr: The kHeapVaddr of the process
	heapOffset: The relevant memory address minus the userland heap base
	returns: The kernel virtual address of the relevant memory
*/
extern uint32_t procHeapToKVaddr(uint32_t kHeapVaddr, uint32_t heapOffset);
