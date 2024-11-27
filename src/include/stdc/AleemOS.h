//AleemOS.h
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
#include <stdint.h>

#ifndef __STDC_ALEEMOS_H
#define __STDC_ALEEMOS_H

int blink(int x, int y);
int cursorPos(void);
void _ls(const char* path);

extern uint32_t _syscall(uint32_t call, uint32_t params);
#endif
