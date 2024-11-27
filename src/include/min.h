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

#define BPB 0x7c00
#define FAT_BASE 0x400000
#define DATA_BASE 0x482000
#define KERNEL_BASE 0xD600
#define DEFAPP_BASE 0x1000

extern void min(void);

extern uint8_t strcmp(uint8_t* f, uint8_t* s);
