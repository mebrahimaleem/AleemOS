//sh.c
//
//Shell for AleemOS

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
