//portio.h
//
//This file has declarations of functions that interact with IO ports


/*
	Sends data to IO port
	port: Address of IO port
	byte: Data to send
*/
extern void outb(uint16_t port, uint8_t byte);

/*
	Gets data from IO port
	port: Address of IO port

	returns data from port
*/
extern uint8_t inb(uint16_t port);

/*
	Sends data to IO port
	port: Address of IO port
	word: Data to send
*/
extern void outw(uint16_t port, uint16_t word);

/*
	Gets data from IO port
	port: Address of IO port

	returns data from port
*/
extern uint16_t inw(uint16_t port);

/*
	Sends data to IO port
	port: Address of IO port
	dword: Data to send
*/
extern void outd(uint16_t port, uint32_t dword);

/*
	Gets data from IO port
	port: Address of IO port

	returns data from port
*/
extern uint32_t ind(uint16_t port);

/*
	Waits for IO buses
*/
extern void io_wait(void);

