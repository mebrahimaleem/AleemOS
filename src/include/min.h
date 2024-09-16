#define BPB 0x7c00
#define FAT_BASE 0x400000
#define DATA_BASE 0x482000
#define KERNEL_BASE 0xD600
#define DEFAPP_BASE 0x1000

extern void min(void);

extern uint8_t strcmp(uint8_t* f, uint8_t* s);
