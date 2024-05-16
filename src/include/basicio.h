//basicio.h
//
//IO libraries for kernel using VGA

/*
	Clears the current screen
*/
extern void clearVGA(void);

/*
	Puts a character at the desired x,y coordinate on the screen
	str: The character to put
	x: x position on the screen
	y: y position on the scrren
	color: The color of the character in VGA text mode color format
*/
extern void put(const char str, uint8_t x, uint8_t y, uint8_t color);

/*
	Prints a string
	str: The string the print
	col: THe color of the string in VGA text mode color format
*/
extern void vgaprint(const char* str, uint8_t col);

/*
	Prints an unsigned integer
	num: The number to print
	base: The base to print the number in
	col: The color of the number in VGA text mode color format
*/
extern void vgaprintint(uint32_t num, uint8_t base, uint8_t col);

/*
	Prints a character
	c: The character to string
	col: The color of the character in VGA text mode color format
*/
extern void vgaprintchar(uint8_t c, uint8_t col);

/*
	The current address in memory to draw the next character
*/
extern uint8_t* volatile vgacursor;

/*
	The lowest character index to which text can be deleted to (using the \b escape character)
*/
extern uint32_t backslock;
