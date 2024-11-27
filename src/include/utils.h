//utils.h
//
//provides declarations for generic utilities in AleemOS
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

/*
	Converts an int32_t to string
	num: The number to convert
	base: The base of the number in string format

	returns a pointer to the null terminates string
*/
extern uint8_t* int32_to_string(int32_t num, uint8_t base);

/*
	Converts an uint32_t to string
	num: The number to convert
	base: The base of the number in string format

	returns a pointer to the null terminates string
*/
extern uint8_t* uint32_to_string(uint32_t num, uint8_t base);

/*
	Sleeps for a certain period of time
	time: Number of milliseconds to sleep
*/
extern void sleepms(uint32_t time);

/*
	Compares two string
	str1: First string
	str2: Second string

	See implementation for return value
*/
extern uint8_t strcmp(uint8_t* str1, uint8_t* str2);

/*
	Gets the number of charavters in a string
	str: The string

	returns the number of characters
*/
extern uint32_t strlen(uint8_t* str);

extern uint8_t* strcpy(uint8_t* dest, const uint8_t* src);
