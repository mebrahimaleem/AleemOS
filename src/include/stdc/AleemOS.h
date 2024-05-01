//AleemOS.h
#include <stdint.h>

#ifndef __STDC_ALEEMOS_H
#define __STDC_ALEEMOS_H

int blink(int x, int y);
int cursorPos(void);

extern uint32_t _syscall(uint32_t call, uint32_t params);
#endif
