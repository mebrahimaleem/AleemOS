//kbd.c
//
//provides implementations for the keyboard driver

#include <stdint.h>
#include "kbd.h"
#include "../kernel/portio.h"
#include "../kernel/memory.h"

volatile KBDEventQueue* volatile KBDRoot = 0;
volatile KBDEventQueue* volatile KBDNextEvent = 0;

typedef struct Keystroke {
	uint8_t E0 : 1;
	uint8_t E1 : 1;
	uint8_t lShft : 1;
	uint8_t rShft : 1;
	uint8_t capLk : 1;
	uint8_t numLk : 1;
	uint8_t scrLk : 1;
} Keystroke;

Keystroke keystroke;

inline void KBDResetMods(){
	keystroke.lShft = 0;
	keystroke.rShft = 0;
	keystroke.capLk = 0;
	keystroke.numLk = 0;
	keystroke.scrLk = 0;
}

void ISR21_handler(uint32_t byte){
	//First check if root is not already set
	if (KBDRoot == 0){
		keystroke.E0 = 0;
		keystroke.E1 = 0;
	}

	//Check special conditions
	switch (byte){
		case 0x3A:
			keystroke.capLk = keystroke.capLk == 0 ? 1 : 0;
			break;
		case 0x46:
			keystroke.scrLk = keystroke.scrLk == 0 ? 1 : 0;
			break;
		case 0xC5:
			keystroke.numLk = keystroke.numLk == 0 ? 1 : 0;
			break;
		case 0x2A:
			keystroke.lShft = 1;
			break;
		case 0xAA:
			keystroke.lShft = 0;
			break;
		case 0x36:
			keystroke.rShft = 1;
			break;
		case 0xB6:
			keystroke.rShft = 0;
			break;
		case 0xE0:
			keystroke.E0 = 1;
			return;
		case 0xE1:
			keystroke.E1 = 1;
			return;
		default:
			if (keystroke.E1 == 1 && (byte != 0xC5)) return;
			break;
	}
	
	if (KBDRoot == 0 || KBDNextEvent == 0){
		KBDRoot = (KBDEventQueue*)malloc(sizeof(KBDEventQueue));
		KBDRoot->keyCode = 0;
		KBDRoot->next = 0;
		KBDRoot->prev = 0;
		KBDNextEvent = KBDRoot;
	}
	else{
		volatile KBDEventQueue* volatile tmp = KBDRoot;
		KBDRoot->prev = (KBDEventQueue*)malloc(sizeof(KBDEventQueue));
		KBDRoot = KBDRoot->prev;
		KBDRoot->next = tmp;
	}

	//Create 16bit keycode
	KBDRoot->keyCode = (uint16_t)(byte + (keystroke.E0 == 1 ? 32768 : 0) + (keystroke.E1 == 1 ? 8192 : 0 ));

	keystroke.E0 = 0;
	keystroke.E1 = 0;
	return;
}

inline char toAscii(uint32_t keycode){
	if (keystroke.lShft == 1 || keystroke.rShft == 1 || keystroke.capLk == 1)
		switch (keycode){
			case 0x1E:
				return 'A';
			case 0x30:
				return 'B';
			case 0x2E:
				return 'C';
			case 0x20:
				return 'D';
			case 0x12:
				return 'E';
			case 0x21:
				return 'F';
			case 0x22:
				return 'G';
			case 0x23:
				return 'H';
			case 0x17:
				return 'I';
			case 0x24:
				return 'J';
			case 0x25:
				return 'K';
			case 0x26:
				return 'L';
			case 0x32:
				return 'M';
			case 0x31:
				return 'N';
			case 0x18:
				return 'O';
			case 0x19:
				return 'P';
			case 0x10:
				return 'Q';
			case 0x13:
				return 'R';
			case 0x1F:
				return 'S';
			case 0x14:
				return 'T';
			case 0x16:
				return 'U';
			case 0x2F:
				return 'V';
			case 0x11:
				return 'W';
			case 0x2D:
				return 'X';
			case 0x15:
				return 'Y';
			case 0x2C:
				return 'Z';
			default:
				return 0; //Unkown keyscan (maybe wrong scan set)
		}

	if (keystroke.lShft == 0 && keystroke.rShft == 0 && keystroke.capLk == 0)
		switch (keycode){
			case 0x1E:
				return 'a';
			case 0x30:
				return 'b';
			case 0x2E:
				return 'c';
			case 0x20:
				return 'd';
			case 0x12:
				return 'e';
			case 0x21:
				return 'f';
			case 0x22:
				return 'g';
			case 0x23:
				return 'h';
			case 0x17:
				return 'i';
			case 0x24:
				return 'j';
			case 0x25:
				return 'k';
			case 0x26:
				return 'l';
			case 0x32:
				return 'm';
			case 0x31:
				return 'n';
			case 0x18:
				return 'o';
			case 0x19:
				return 'p';
			case 0x10:
				return 'q';
			case 0x13:
				return 'r';
			case 0x1F:
				return 's';
			case 0x14:
				return 't';
			case 0x16:
				return 'u';
			case 0x2F:
				return 'v';
			case 0x11:
				return 'w';
			case 0x2D:
				return 'x';
			case 0x15:
				return 'y';
			case 0x2C:
				return 'z';
			default:
				return 0; //Unkown keyscan (maybe wrong scan set)
		}

	return 0; //Invalid keyscan
}

