//ELFparse.h
//
//This file provides declarations for reading ELF files

typedef uint32_t Elf32Addr;
typedef uint16_t Elf32Half;
typedef uint32_t Elf32Offs;
typedef int32_t Elf32Sword;
typedef uint32_t Elf32Word;

typedef struct Elf32indent{
	uint8_t fid [4];
	uint8_t cls;
	uint8_t encd;
	uint8_t ver;
	uint8_t pad [9];
} Elf32indent;

typedef struct Elf32header {
	Elf32indent indent;
	Elf32Half type;
	Elf32Half machine;
	Elf32Word version;
	Elf32Addr entry;
	Elf32Offs phoff;
	Elf32Offs shoff;
	Elf32Word flags;
	Elf32Half ehsize;
	Elf32Half phentsize;
	Elf32Half phnum;
	Elf32Half shentsize;
	Elf32Half shnum;
	Elf32Half shstrndx;
} Elf32header;

typedef struct Elf32sheader {
	Elf32Word name;
	Elf32Word type;
	Elf32Word flags;
	Elf32Addr addr;
	Elf32Offs offs;
	Elf32Word size;
	Elf32Word link;
	Elf32Word info;
	Elf32Word align;
	Elf32Word esize;
} Elf32sheader;

typedef struct Elf32pheader {
	Elf32Word type;
	Elf32Offs offs;
	Elf32Addr vaddr;
	Elf32Addr paddr;
	Elf32Word fsize;
	Elf32Word msize;
	Elf32Word flags;
	Elf32Word align;
} Elf32pheader;

typedef struct ElfPageList {
	uint32_t paddr;
	uint32_t vaddr;
	struct ElfPageList* next;
} ElfPageList;

extern uint8_t parseElf(uint8_t* volatile src, Elf32sheader** shead, Elf32pheader** phead, Elf32Half* snum, Elf32Half* pnum, uint8_t** strtbl, uint32_t* entry);

extern uint8_t calcELF(uint8_t* volatile src, ElfPageList** pages, uint32_t* size);

extern uint8_t copyELF(uint8_t* volatile src, uint8_t* volatile dst, uint32_t* volatile ent);
