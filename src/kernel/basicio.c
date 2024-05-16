//basicio.c
//
//IO library for the kernel using VGA

#include <stdint.h>
#include <basicio.h>
#include <memory.h>
#include <utils.h>

uint8_t* volatile vgacursor = (uint8_t* volatile)0xb8000;
uint32_t backslock = 0xb8000;

inline void clearVGA(){
	for (uint8_t* volatile i = (uint8_t* volatile)0xb8000; (uint32_t)i < 0xb8000 + 25 * 80 * 2; i += 2) *i = 0;
	for (uint8_t* volatile i = (uint8_t* volatile)0xb8001; (uint32_t)i < 0xb8000 + 25 * 80 * 2; i += 2) *i = 0x0F;
	vgacursor = (uint8_t* volatile)0xb8000;
}

inline void put(const char str, uint8_t x, uint8_t y, uint8_t color){
	*(char* volatile)(0xb8000 + (x + y * 80)*2) = str;
	*(uint8_t* volatile)(0xb8000 + (x + y * 80)*2 + 1) = color;
	return;
}

void vgaprint(const char* str, uint8_t col){
	for (uint8_t* volatile i = (uint8_t* volatile)str; *i != 0; i++){
		//Check if we need to scroll
		if ((uint32_t)vgacursor == 0xb8000 + (25 * 80 * 2)){
			for (uint8_t* volatile j = (uint8_t* volatile)(0xb8000 + 80 * 2); (uint32_t)j < 0xb8000 + 80 * 25 * 2; j++) *(j - 160) = *j; //Scroll
			for (uint8_t* volatile j = (uint8_t* volatile)(0xb8000 + 80 * 24 * 2); (uint32_t)j < 0xb8000 + 80 * 25 * 2; j += 2) *j = 0; //Clear last line
			for (uint8_t* volatile j = (uint8_t* volatile)(0xb8001 + 80 * 24 * 2); (uint32_t)j < 0xb8000 + 80 * 25 * 2; j += 2) *j = 0x0F;
			vgacursor -= 160; //Move cursor
			backslock -= 160;
			if (backslock < 0xb8000) backslock = 0xb8000;
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
		 	vgacursor = (uint8_t* volatile)((vgacursor - 0xb8000) - (((uint32_t)vgacursor - 0xb8000) % 160) + 160 + 0xb8000); //Newline
			//Check if we need to scroll
			if ((uint32_t)vgacursor == 0xb8000 + (25 * 80 * 2)){
				for (uint8_t* volatile j = (uint8_t* volatile)(0xb8000 + 80 * 2); (uint32_t)j < 0xb8000 + 80 * 25 * 2; j++) *(j - 160) = *j; //Scroll
				//Clear last line
				for (uint8_t* volatile j = (uint8_t* volatile)(0xb8000 + 80 * 24 * 2); (uint32_t)j < 0xb8000 + 80 * 25 * 2; j += 2 ) *j = 0;
				for (uint8_t* volatile j = (uint8_t* volatile)(0xb8001 + 80 * 24 * 2); (uint32_t)j < 0xb8000 + 80 * 25 * 2; j += 2) *j = 0x0F;
				vgacursor -= 160; //Move cursor
				backslock -= 160;
				if (backslock < 0xb8000) backslock = 0xb8000;
			}
		}

		else if (*i == '\r') ; //Ignore
		else if (*i == '\b'){ //Backspace
			if ((uint32_t)vgacursor > backslock){
				*(--vgacursor) = 0x0F;
				*(--vgacursor) = 0;
				if ((uint32_t)vgacursor < 0xb8000) vgacursor = (uint8_t* volatile)0xb8000;
				else if (((uint32_t)vgacursor - 0xb8000) % 160 == 158) while ((uint32_t)(*(vgacursor-2) == 0 && (uint32_t)vgacursor > 0xb8000)) vgacursor -= 2;
			}
		}

		else if (*i == 0x11){ //Left arrow
			while (1){
				vgacursor -= 2;
				if ((uint32_t)vgacursor < backslock){
					vgacursor = (uint8_t* volatile)backslock;
					break;
				}
				if (*vgacursor != 0) break;
			}
		}
		else if (*i == 0x10 && (uint32_t)vgacursor + 2 < 0xb8000 + 80 * 25 * 2 && *vgacursor != 0){ //Right arrow
			vgacursor += 2;
		}
		else if (*i == 0x1E) { //Up arrow
			if ((uint32_t)vgacursor - 160 >= backslock)
				vgacursor = (uint8_t* volatile)((vgacursor - 0xb8000) - (((uint32_t)vgacursor - 0xb8000) % 160) - 160 + 0xb8000);
		}
		else if (*i == 0x1F || *i == 0x10) { //Down arrow (or right arrow if it could not resolve)
			for (uint8_t j = 1; (uint32_t)vgacursor + 160 * j < 0xb8000 + 25 * 80 + 2; j++)
				if (
				*((uint8_t* volatile)((vgacursor - 0xb8000) - (((uint32_t)vgacursor - 0xb8000) % 160) + 160 * j + 0xb8000)) != 0)
					vgacursor = (uint8_t* volatile)((vgacursor - 0xb8000) - (((uint32_t)vgacursor - 0xb8000) % 160) + 160 + 0xb8000);
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

inline void vgaprintint(uint32_t num, uint8_t base, uint8_t col){
	char* str = (char* )uint32_to_string(num, base);
	vgaprint(str, col);
	free(str);
}

void vgaprintchar(uint8_t c, uint8_t col){
	uint8_t* s = malloc(2);
	s[0] = c;
	s[1] = 0;
	vgaprint((const char*)s, col);
	free(s);
	return;
}
