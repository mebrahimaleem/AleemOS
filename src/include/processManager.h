
/*
	Called to manage intel reserved IRQs
	check: The interrupt code (see implementation in kernel/kentry.asm)
	cs: cs register value
*/
void processManager(uint32_t check, uint32_t cs);

/*
	Called to bridge userland to kernel
	call: System call number (see implementation)
	params: Pointer to parameter in userland (or the parameter itself)
	cs: cs register value
*/
uint32_t sysCall(uint32_t call, uint32_t params, uint32_t cs);
