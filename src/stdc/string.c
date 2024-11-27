//string.c
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
