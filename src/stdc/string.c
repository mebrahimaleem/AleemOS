//string.c

#ifndef __STDC_STRING_C
#define __STDC_STRING_C
#include "string.h"


int strcmp(const char* str1, const char* str2){
	const char* s1 = str1, *s2 = str2;
	while (1){
		if (*s1 == 0 && *s2 == 0) return 0;
		if (*s1 != * s2) return *s1 > *s2 ? 1 : -1;
		s1++;
		s2++;
	}
}

size_t strlen(const char* str){
	unsigned int s = 0;
	for (const char* i = str; *i != 0; i++) s++;
	return s;
}

char* strcpy(char* dest, const char* src){
	char* i = dest;
	for (const char* j = src; *j != 0; j++){
		*i = *j;
		i++;
	} 
	*i = 0;
	return dest;
}
#endif
