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

#include <portio.h>
#include <basicio.h>
#include <processManager.h>
#include <kernel.h>
#include <process.h>
#include <kbd.h>
#include <boot2.h>
#include <processScheduler.h>

uint32_t last_exitcode = 0;

//sysCall is meant for handling system calls from userland
uint32_t sysCall(uint32_t call, uint32_t params){
	uint32_t ret = 0;
	switch (call){
		case 0: //kill(PID)
			ret = killProcess(params);
			break;
		case 1: //printchar(char)
			vgaprintchar((uint8_t)params, 0x0F);
			params = (uint32_t)(vgacursor - (uint8_t*)0xb8000)/2;
			backslock = (uint32_t)vgacursor;
			goto blink;
		blink:
		case 2: //blink(addr)
			outb(0x3D4, 0x0F);
			outb(0x3D5, (uint8_t)(params & 0xFF));
			outb(0x3D4, 0x0E);
			outb(0x3D5, (uint8_t)((params >> 8) & 0xFF));
			break;
		case 3: //getchar()
			setKBDEventTrack(1);
			while (1){
				if (KBDNextEvent != 0){
					ret = (uint32_t)toAscii(KBDNextEvent->keyCode);
					KBDNextEvent = KBDNextEvent->next;
					if (ret != 0){
						vgaprintchar((uint8_t)ret, 0x0F);
						params = (uint32_t)(vgacursor - (uint8_t*)0xb8000)/2;
						setKBDEventTrack(0);
						goto blink;
					}
				}
				asm volatile ("sti \n hlt \n cli" : : : "memory");
				continue;
			}
			break;
		case 4: //getcursorpos()
			ret = (uint32_t)(vgacursor - (uint8_t* )0xb8000)/2;
			break;
		case 5: //lockcursor()
			backslock = (uint32_t)vgacursor;
			break;
		case 6: //printstring(heapOffset)
			vgaprint((const char*)procHeapToKVaddr(_schedulerCurrentProcess->kHeapVaddr, params), 0x0F);
			params = (uint32_t)(vgacursor - (uint8_t*)0xb8000)/2;
			backslock = (uint32_t)vgacursor;
			goto blink;
			break;
		case 8: //pid() - returns processes PID
			ret = _schedulerCurrentProcess->PID;
			break;
		default:
			break;
	}

	return ret;
}

//processManager is meant for handling exception caused interupts, IRQs
void processManager(uint32_t check){
	//Check for kill process
	if ((check & 0x20) == 0x20){
		last_exitcode = check;
		vgaprint("An exception has occured! Error code: ", 0x40);
		vgaprintint(check - 0x20, 16, 0x40);
		vgaprint("\n", 0x40);
		killProcess(_schedulerCurrentProcess->PID);
	}

	//Check for IRQ1
	else if (check == 2) {
		uint32_t stroke;
		asm volatile ("push eax \n in al, 0x60 \n movzx eax, al \n mov %0, eax \n pop eax" : "=m"(stroke): : "memory"); //Get keystroke
		ISR21_handler(stroke); //Pass keystroke to driver
		asm volatile ("push eax \n mov al, 0x20 \n out 0x20, al \n pop eax" : : : "memory"); //Tell PIC we are done
	}

	//Check for Master PIC IRQ
	else if (check <= 7){
		asm volatile ("push eax \n mov al, 0x20 \n out 0x20, al \n pop eax" : : : "memory"); //Tell PIC we are done
	}

	//Check for spurious
	else if (check == 8){
		uint32_t res;
		asm volatile ("push eax \n mov al, 0xb \n out 0x20, al \n xor al, al \n out 0x80, al \n in al, 0x20 \n movzx eax, al \n \
				mov %0, eax \n pop eax" : "=m"(res) : : "memory");
		if (res == 7)
			asm volatile ("push eax \n mov al, 0x20 \n out 0x20, al \n pop eax" : : : "memory");
	}

	//Check for Slave PIC IRQ
	else if (check <= 15){
		asm volatile ("push eax \n mov al, 0x20 \n out 0x20, al \n out 0xA0, al \n pop eax"); //Tell PIC we are done
	}

	//Check for spurious
	else if (check == 16){
		uint32_t res;
		asm volatile ("push eax \n mov al, 0xb \n out 0xA0, al \n xor al, al \n out 0x80, al \n in al, 0xA0 \n movzx eax, al \n \
				mov %0, eax \n pop eax" : "=m"(res) : : "memory");
		if (res == 15)
			asm volatile ("push eax \n mov al, 0x20 \n out 0xA0, al \n pop eax" : : : "memory");
		asm volatile ("push eax \n mov al, 0x20 \n out 0x20, al \n pop eax" : : : "memory");
	}

	return;
}

inline uint32_t procHeapToKVaddr(uint32_t kHeapVaddr, uint32_t heapOffset) {
	return kHeapVaddr + heapOffset;
}
