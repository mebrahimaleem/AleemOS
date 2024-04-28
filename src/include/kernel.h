//kernel.h
//
//provides declarations for shared kernel data structures and functions

//#define KERNEL_DEBUG

extern volatile uint16_t k_KDATA;

/*
	Strucutre to store information on used memory (from the BIOS)
*/
typedef struct MemoryMapEntry {
	volatile uint64_t base; // Start of memory
	volatile uint64_t size; // Size of memory
	volatile uint32_t type; // Memory type (see BIOS docs or implementation)
} MemoryMapEntry;

/*
	Structure to store information on time since some point during boot
*/
typedef struct SystemTime {
	volatile uint32_t fraction_ms; // Fractional milliseconds
	volatile uint32_t whole_ms; // Whole milliseconds
	volatile uint32_t fraction_diff; // Fractional diff (used by PIT IRQ)
	volatile uint32_t whole_diff; // Whole diff (used by PIT IRQ)
} SystemTime;

/*
	Strucuture to hold information when transferring from assembly to C
*/
typedef struct KernelData {
	volatile uint32_t MBR_SIG; // MBR timestamp signature (for finding boot device)
	volatile MemoryMapEntry mmape[16]; // Memory map
	volatile SystemTime systemTime; // System time
} KernelData;

/*
	Structure for holding GDT pointer
*/
typedef struct GDT_ptr {
	uint16_t size; // Size of GDT
	uint32_t addr; // Address of GDT
} GDT_ptr;

/*
	Structure for holding LDT pointer
*/
typedef struct LDT_ptr {
	uint16_t flg : 3; // Flags
	uint16_t seg : 13; //Segment index in GDT
} __attribute((packed)) LDT_ptr;

/*
	Structure for holding IDT pointer
*/
typedef struct IDT_ptr {
	uint16_t size; //Size of IDT
	uint32_t addr; //Address of IDT
} IDT_ptr;

/*
	GDT or LDT Entry
	Read Intel docs for better description
*/
typedef struct DTentry {
	uint16_t lim0; // Limit
	uint16_t bas0; // Starting address
	uint8_t bas1; // Starting address
	uint8_t type : 5; // b11010C b10010D b00010L b01001T
	uint8_t DPL : 2; // Privilage level
	uint8_t present : 1;
	uint8_t lim1 : 4; //Limit
	uint8_t AVL : 1;
	uint8_t flg : 2; // "10"
	uint8_t granularity : 1;
	uint8_t bas2; // Starting address
} __attribute((packed)) DTentry;

/*
	IDT Task Gate Structure
*/
typedef struct IDTtask {
	uint16_t r0;
	uint16_t selector;
	uint8_t r1;
	uint8_t type : 5; // "00101"
	uint8_t DPL : 2;
	uint8_t present : 1;
	uint16_t r2;
} __attribute((packed)) IDTtask;

/*
	IDT Interrupt Gate Structure
*/
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

/*
	IDT Trap Gate Strucutre
*/
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

/*
	TSS Strucutre
*/
typedef struct TSS {
	uint16_t link; // GDT index to previous TSS
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
	uint16_t IOPB; // NOTE: maybe implement this for less overhead on system calls (ex: allow userland to move VGA cursor)
} TSS;

/*
	Kernel Data
*/
extern volatile KernelData* volatile kdata;
extern volatile TSS* volatile kTSS;

/*
	GDT
*/
extern DTentry* volatile GDT;
extern struct PCIEntry* pciEntries;
