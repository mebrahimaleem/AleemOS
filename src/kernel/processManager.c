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

#pragma GCC push_options
#pragma GCC optimize("O0")

//sysCall is meant for handling system calls from userland
uint32_t sysCall(uint32_t cs, uint32_t call, uint32_t params){
	uint32_t ret = 0;
	switch (call){
		case 0: //kill(exitcode)
			last_exitcode = params;
			killProcess();
			break;
		case 1: //printchar(char)
			vgaprintchar((uint8_t)params, 0x0F);
			params = (uint32_t)(vgacursor - (volatile uint8_t*)0xb8000)/2;
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
			break;
			setKBDEventTrack(1);
			while (1){
				if (KBDNextEvent != 0){
					ret = (uint32_t)toAscii(KBDNextEvent->keyCode);
					KBDNextEvent = KBDNextEvent->next;
					if (ret != 0){
						vgaprintchar((uint8_t)ret, 0x0F);
						params = (uint32_t)(vgacursor - (volatile uint8_t*)0xb8000)/2;
						setKBDEventTrack(0);
						goto blink;
					}
				}
				asm volatile ("sti \n hlt \n cli" : : : "memory");
				continue;
			}
		case 4: //getcursorpos()
			ret = (uint32_t)(vgacursor - (volatile uint8_t* volatile)0xb8000)/2;
			break;
		default:
			break;
	}

	//We need to execute a far return
	asm volatile ("mov eax, %0 \n leave \n retf" : : "m"(ret), "m"(cs) : "memory");
	return 0;
}

//processManager is meant for handling exception caused interupts, IRQs
void processManager(uint32_t cs, uint32_t check){
	//Check for kill process
	if ((check & 0x20) == 0x20){
		last_exitcode = check;
		vgaprint((volatile char* volatile)"An exception has occured! Error code: ", 0x40);
		vgaprintint(check, 16, 0x40);
		vgaprint((volatile char* volatile)"\n", 0x40);
		killProcess();
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

	//We need to execute a far return
	asm volatile ("leave \n retf" : : "m"(cs) : "memory");
}
#pragma GCC pop_options
