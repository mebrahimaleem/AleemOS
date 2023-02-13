//kernel.h
//
//provides declarations for shared kernel data structures and functions

void kernel(void);
void processManager(uint32_t check, uint32_t cs);
uint32_t sysCall(uint32_t call, uint32_t params, uint32_t cs);

typedef struct bool1 {
	uint8_t b : 1;
} bool1;

typedef struct MemoryMapEntry {
	volatile uint64_t base;
	volatile uint64_t size;
	volatile uint32_t type;
} MemoryMapEntry;

typedef struct SystemTime {
	volatile uint32_t fraction_ms;
	volatile uint32_t whole_ms;
	volatile uint32_t fraction_diff;
	volatile uint32_t whole_diff;
} SystemTime;

typedef struct KernelData {
	volatile uint32_t MBR_SIG;
	volatile MemoryMapEntry mmape[16];
	volatile SystemTime systemTime;
} KernelData;

typedef struct GDT_ptr {
	uint16_t size;
	uint32_t addr;
} GDT_ptr;

typedef struct LDT_ptr {
	uint16_t flg : 3;
	uint16_t seg : 13;
} __attribute((packed)) LDT_ptr;

typedef struct IDT_ptr {
	uint16_t size;
	uint32_t addr;
} IDT_ptr;

typedef struct TSS_ptr {
	uint16_t seg;
} TSS_ptr;

typedef struct DTentry {
	uint16_t lim0;
	uint16_t bas0;
	uint8_t bas1;
	uint8_t type : 5; // b11010C b10010D b00010L b01001T
	uint8_t DPL : 2;
	uint8_t present : 1;
	uint8_t lim1 : 4;
	uint8_t AVL : 1;
	uint8_t flg : 2; // "10"
	uint8_t granularity : 1;
	uint8_t bas2;
} __attribute((packed)) DTentry;

typedef struct IDTtask {
	uint16_t r0;
	uint16_t selector;
	uint8_t r1;
	uint8_t type : 5; // "00101"
	uint8_t DPL : 2;
	uint8_t present : 1;
	uint16_t r2;
} __attribute((packed)) IDTtask;

typedef struct IDTint {
	uint16_t offset0;
	uint16_t selector;
	uint8_t r0 : 4;
	uint8_t zero : 4;
	uint8_t type : 5; // "01110"
	uint8_t DPL : 2;
	uint8_t present : 1;
	uint16_t offset1;
} __attribute((packed)) IDTint;

typedef struct IDTtrap {
	uint16_t offset0;
	uint16_t selector;
	uint8_t r0 : 4;
	uint8_t zero : 4;
	uint8_t type : 5; // "01111"
	uint8_t DPL : 2;
	uint8_t present : 1;
	uint16_t offset1;
} __attribute((packed)) IDTtrap;

typedef struct TSS {
	uint16_t link;
	uint16_t r0;
	uint32_t esp0;
	uint16_t ss0;
	uint16_t r1;
	uint32_t esp1;
	uint16_t ss1;
	uint16_t r2;
	uint32_t esp2;
	uint16_t ss2;
	uint16_t r3;
	uint32_t cr3;
	uint32_t eip;
	uint32_t eflags;
	uint32_t eax;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t esp;
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;
	uint16_t es;
	uint16_t r4;
	uint16_t cs;
	uint16_t r5;
	uint16_t ss;
	uint16_t r6;
	uint16_t ds;
	uint16_t r7;
	uint16_t fs;
	uint16_t r8;
	uint16_t gs;
	uint16_t r9;
	uint16_t ldtd;
	uint16_t r10;
	uint16_t r11;
	uint16_t IOPB;
} TSS;

extern volatile KernelData* volatile kdata;
extern TSS* volatile KTSS;
extern TSS* volatile UTSS;
extern DTentry* volatile GDT;
