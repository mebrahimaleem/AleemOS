//basicio.c
//
//IO library for the kernel using VGA

#include <stdint.h>
#include "basicio.h" 

volatile uint8_t* volatile vgacursor = (volatile uint8_t* volatile)0xb8000;

inline void clearVGA(){
	for (volatile uint8_t* volatile i = (volatile uint8_t* volatile)0xb8000; (uint32_t)i < 0xb8000 + 25 * 80 * 2; i++) *i = 0;
	vgacursor = (volatile uint8_t* volatile)0xb8000;
}

inline void put(volatile char str, volatile uint8_t x, volatile uint8_t y, volatile uint8_t color){
	*(volatile char* volatile)(0xb8000 + (x + y * 80)*2) = str;
	*(volatile uint8_t* volatile)(0xb8000 + (x + y * 80)*2 + 1) = color;
	return;
}

inline void vgaprint(volatile char* volatile str, volatile uint8_t col){
	for (volatile uint8_t* volatile i = (volatile uint8_t* volatile)str; *i != 0; i++){
		//Check if we need to scroll
		if ((uint32_t)vgacursor == 0xb8000 + (25 * 80 * 2)){
			for (volatile uint8_t* volatile j = (volatile uint8_t* volatile)(0xb8000 + 80 * 2); (uint32_t)j < 0xb8000 + 80 * 25 * 2; j++) *(j - 160) = *j; //Scroll
			for (volatile uint8_t* volatile j = (volatile uint8_t* volatile)(0xb8000 + 80 * 24 * 2); (uint32_t)j < 0xb8000 + 80 * 25 * 2; j++) *j = 0; //Clear last line
			vgacursor -= 160; //Move cursor
		}

		//Process escape codes
		if (*i == '\t'){ //Tab
			for (uint8_t j = 0; j < 4; j++){
				*vgacursor = ' ';
				*(++vgacursor) = col;
				vgacursor++;
			}
		}
		else if (*i == '\n'){
		 	vgacursor = (volatile uint8_t* volatile)((vgacursor - 0xb8000) - (((uint32_t)vgacursor - 0xb8000) % 160) + 160 + 0xb8000); //Newline
			//Check if we need to scroll
			if ((uint32_t)vgacursor == 0xb8000 + (25 * 80 * 2)){
				for (volatile uint8_t* volatile j = (volatile uint8_t* volatile)(0xb8000 + 80 * 2); (uint32_t)j < 0xb8000 + 80 * 25 * 2; j++) *(j - 160) = *j; //Scroll
				for (volatile uint8_t* volatile j = (volatile uint8_t* volatile)(0xb8000 + 80 * 24 * 2); (uint32_t)j < 0xb8000 + 80 * 25 * 2; j++) *j = 0; //Clear last line
				vgacursor -= 160; //Move cursor
			}
		}

		else if (*i == '\r') ; //Ignore
		else if (*i == '\b'){ //Backspace
			*(--vgacursor) = 0;
			*(--vgacursor) = 0;
			if ((uint32_t)vgacursor < 0xb8000) vgacursor = (volatile uint8_t* volatile)0xb8000;
			else if (((uint32_t)vgacursor - 0xb8000) % 160 == 158) while ((uint32_t)(*(vgacursor-2) == 0 && (uint32_t)vgacursor > 0xb8000)) vgacursor -= 2;
		}

		//Otherwise print the character
		else{
			*vgacursor = *i;
			*(++vgacursor) = col;
			vgacursor++;
		}
	}

	return;
}
