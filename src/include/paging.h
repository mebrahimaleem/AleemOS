
extern uint32_t*** kernelPD;

extern void initPaging(void);
extern uint8_t mapMemory(uint32_t*** PD, uint32_t vaddr, uint32_t paddr, uint8_t flg);
extern uint8_t mapMemory4M(uint32_t*** PD, uint32_t vaddr, uint32_t paddr, uint8_t flg);
extern uint32_t* allocPagingStruct(void);
