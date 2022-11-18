//utils.c
//
//provides implementations for generic utilities in AleemOS

#include <stdint.h>
#include "kernel.h"
#include "memory.h"
#include "utils.h"

extern volatile KernelData* volatile kdata;

//Converts an integer to string
inline uint8_t* int32_to_string(int32_t num, uint8_t base){

	//Check for number 0 or invalid base
	if (num == 0 || base < 2 || base > 36){
		uint8_t* str = (uint8_t*)malloc(2);
		str[0] = '0';
		str[1] = 0;
		return str;
	}

	//Check for negative number
	uint8_t* str = (uint8_t*)malloc(32);
	uint8_t neg = 0;
	if (num < 0){
		neg = 1;
		num = -num;
	}
	
	//Do conversion
	for (uint8_t i = 0; i < 32; i++){
		str[31-i] = (uint8_t)(num % base);
		num /= base;
	}

	//Make string better looking (remove leading zeros)
	uint8_t* st = 0;
	uint8_t* s = 0;
	for (uint8_t* i = str; i < str + 32; i++){
		if (*i != 0 && st == 0){
			st = (uint8_t*)malloc((uint8_t)(33 - (i - str) + (neg ? 1 : 0)));
			s = st;
			if (neg == 1){
				st[0] = '-';
				st++;
			}
			st[32 - (i - str)] = 0;
		}
		else if (st == 0) continue;

		//Check for letter or numeric digit
		if (*i < 10) *st = '0' + *i;
		else *st = 'A' - 10 + *i;
		st++;
	}
	
	free(str);
	return s;
}

//Converts a uint32_t to string
inline uint8_t* uint32_to_string(uint32_t num, uint8_t base){

	//Check for number 0 or invalid base
	if (num == 0 || base < 2 || base > 36){
		uint8_t* str = (uint8_t*)malloc(2);
		str[0] = '0';
		str[1] = 0;
		return str;
	}

	uint8_t* str = (uint8_t*)malloc(32);
	//Do conversion
	for (uint8_t i = 0; i < 32; i++){
		str[31-i] = (uint8_t)(num % base);
		num /= base;
	}

	//Make string better looking (remove leading zeros)
	uint8_t* st = 0;
	uint8_t* s = 0;
	for (uint8_t* i = str; i < str + 32; i++){
		if (*i != 0 && st == 0){
			st = (uint8_t*)malloc((uint8_t)(33 - (i - str)));
			s = st;
			st[32 - (i - str)] = 0;
		}
		else if (st == 0) continue;

		//Check for letter or numeric digit
		if (*i < 10) *st = '0' + *i;
		else *st = 'A' - 10 + *i;
		st++;
	}
	
	free(str);
	return s;
}

//Sleeps the procces for time ms
inline void sleepms(uint32_t time){
	volatile uint32_t ctime = kdata->systemTime.whole_ms;
	while (kdata->systemTime.whole_ms - ctime < time) asm volatile ("hlt" : : : "memory");
	return;
}
