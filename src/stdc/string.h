//string.h

#ifndef __STDC_STRING_H
#define __STDC_STRING_H

typedef unsigned int size_t;

int strcmp(const char* str1, const char* str2);
size_t strlen(const char* str);
char* strcpy(char* dest, const char* src);
#endif
