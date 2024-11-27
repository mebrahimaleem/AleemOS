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
