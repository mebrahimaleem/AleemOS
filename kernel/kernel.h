//kernel.h
//
//provides declarations for shared kernel data structures and functions

void kernel(void);

typedef struct MemoryMapEntry {
	volatile uint64_t base;
	volatile uint64_t size;
	volatile uint32_t type;
} MemoryMapEntry;

typedef struct SystemTime {
	volatile uint32_t fraction_ms;
	volatile uint32_t whole_ms;
} SystemTime;

typedef struct KernelData {
	volatile uint32_t MBR_SIG;
	volatile MemoryMapEntry mmape[16];
	volatile SystemTime systemTime;
} KernelData;

