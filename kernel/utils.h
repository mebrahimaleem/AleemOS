//utils.h
//
//provides declarations for generic utilities in AleemOS

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
