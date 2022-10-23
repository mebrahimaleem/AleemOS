//portio.h
//
//This file has declarations of functions that interact with IO ports


extern void outb(volatile uint16_t port, volatile uint8_t byte);
extern volatile uint8_t inb(volatile uint16_t port);

extern void outw(volatile uint16_t port, volatile uint16_t word);
extern volatile uint16_t inw(volatile uint16_t port);

extern void outd(volatile uint16_t port, volatile uint32_t dword);
extern volatile uint32_t ind(volatile uint16_t port);

extern void io_wait(void);

