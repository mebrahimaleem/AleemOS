//sh.c
//
//Shell for AleemOS
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

#include "sh.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <AleemOS.h>

const char* curDir = "~";

int main(int argc, char** argv){
	printf("%s", "AleemOS 0.0.0 IA-32\nPlease see the licensing for AleemOS at https://github.com/mebrahimaleem/AleemOS/blob/main/LICENSE\n\nWelcome to AleemOS Shell\n");
	
	//Check if starting directory was passed
	if (argc >= 2) curDir = argv[1];
	
	printf("root:%s$ ", curDir);

	char c;
	while (1){
		scanf("%c", &c);
	}
	return 0;
}
