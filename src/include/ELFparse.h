//ELFparse.h
//
//This file provides declarations for reading ELF files

typedef uint32_t Elf32Addr; //ELF 32 Address Type
typedef uint16_t Elf32Half; //ELF 32 Halfword type
typedef uint32_t Elf32Offs; //ELF 32 Offset Type
typedef int32_t Elf32Sword; //ELF 32 Signed Word Type
typedef uint32_t Elf32Word; //ELF 32 Unsigned Word Type

/*
	Structure for holding information on the ELF indent
*/
typedef struct Elf32indent{
	uint8_t fid [4]; //File identification magic bytes
	uint8_t cls; //File class
	uint8_t encd; //File encoding
	uint8_t ver; //File version
	uint8_t pad [9]; //Padding
} Elf32indent;

/*
	Structure for holding information on the ELF file header
*/
typedef struct Elf32header {
	Elf32indent indent; //Indent
	Elf32Half type; //Type
	Elf32Half machine; //Machine
	Elf32Word version; //Version
	Elf32Addr entry; //Entry address
	Elf32Offs phoff; //Program header offset
	Elf32Offs shoff; //Section header offset
	Elf32Word flags; //Flags
	Elf32Half ehsize; //ELF file header size
	Elf32Half phentsize; //Size of entries in program header
	Elf32Half phnum; //Number of entries in program header
	Elf32Half shentsize; //Size of entries in section header
	Elf32Half shnum; //Number of entreis in section header
	Elf32Half shstrndx; //Index of string table
} Elf32header;

/*
	Structure for holding information on the ELF section header
*/
typedef struct Elf32sheader {
	Elf32Word name; //Inex in string table for name of section
	Elf32Word type; //Type of section
	Elf32Word flags; //Section flags
	Elf32Addr addr; //Section address in memory
	Elf32Offs offs; //Section offset in file
	Elf32Word size; //Section size
	Elf32Word link; //Linked sections
	Elf32Word info; //Info
	Elf32Word align; //Alignment requirements
	Elf32Word esize; //Entry size
} Elf32sheader;

/*
	Structure for holding information on the ELF program header
*/
typedef struct Elf32pheader {
	Elf32Word type; //Type of program header
	Elf32Offs offs; //Offset in file
	Elf32Addr vaddr; //Virtual address in memory
	Elf32Addr paddr; //Physical address in memory, unused with paging
	Elf32Word fsize; //Size in file
	Elf32Word msize; //Size in memory (important for bss)
	Elf32Word flags; //Flags
	Elf32Word align; //Alignment requirements
} Elf32pheader;

/*
	Structure for holding information on paging requirements for the executable
*/
typedef struct ElfPageList {
	uint32_t paddr; //Physical address mapping
	uint32_t vaddr; //Virtual address mapping
	struct ElfPageList* next; //Pointer to next node in list
} ElfPageList;

/*
	Parses information about an ELF
	src: Address of file in memory
	shead: Pointer to pointer to first section header (modifies memory)
	phead: Pointer to pointer to first program header (modifies memory)
	snum: Pointer to number of section headers (modifies memory)
	pnum: Pointer to number of program headers (modifies memory)
	strtbl: Pointer to address of start of string table (modifies memory)
	entry: Pointer to uint32_t which contains offset into memory of starting address (modifies memory)

	returns status code (see implementation)
	Function will fill the structures pointed by the arguments
*/
extern uint8_t parseElf(uint8_t* volatile src, Elf32sheader** shead, Elf32pheader** phead, Elf32Half* snum, Elf32Half* pnum, uint8_t** strtbl, uint32_t* entry);

/*
	Determines paging information for ELF file
	src: Address of file in memory
	pages: Pointer to first page list node (modifies memory)
	size: Pointer to uint32_t containing total executable size in memory (modifies memory)

	returns status code (see implementation)
	Function will fill out the structures pointed by the arguments
*/
extern uint8_t calcELF(uint8_t* volatile src, ElfPageList** pages, uint32_t* size);

/*
	Copies the neccesary parts of the executable
	src: Address of the file in memory
	dst: Address of the destination executable
	ent: Pointer to uint32_t containing memory address of the entry point (modifies memory)

	returns status code (see implementation)
	Function will fill out the structures pointed by the arguments
*/
extern uint8_t copyELF(uint8_t* volatile src, uint8_t* volatile dst, uint32_t* volatile ent);
