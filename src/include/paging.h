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

#define PAGING_KO 3 //kernel only

extern uint32_t*** kernelPD;

extern void initPaging(void);
extern uint8_t mapMemory(uint32_t*** PD, uint32_t vaddr, uint32_t paddr, uint8_t flg);
extern uint8_t mapMemory4M(uint32_t*** PD, uint32_t vaddr, uint32_t paddr, uint8_t flg);
extern uint32_t* allocPagingStruct(void);
