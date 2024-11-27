//kbd.h
//
//provides declarations for the keyboard driver
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
	Structure for keeping track of keystrokes
*/
typedef struct KBDEventQueue {
	uint16_t keyCode; // System Internal Keycode
	struct KBDEventQueue* next; // Pointer to next node  in queue
	struct KBDEventQueue* prev; // Pointer to previous node in queue
} KBDEventQueue;

/*
	Resets all keyboard modifiers and ongoing keystrokes
	This must be called during kernel setup to ensure correct keycodes
*/
extern void KBDResetMods(void);

/*
	Pointer to latest complete keystroke.
	To use:
		1) disable interrupts
		2) copy KBDNextEvent->keyCode;
		3) set KBDNextEvent = KBDNextEvent->next
*/
extern KBDEventQueue* volatile KBDNextEvent;

/*
	Handler for PS/2 Keyboard IRQ
	byte: the value from IO port 0x60 (PS/2 Keyboard keycode)
*/
extern void ISR21_handler(uint32_t byte);

/*
	Converts System Internal Keycode to Ascii Keycode
	keycode: the the keycode to convert to Ascii

	returns the Ascii equivilant keycode
*/
extern char toAscii(uint32_t keycode);

/*
	Changes the current keyboard tracking setting
	setting: The new keyboard tracking setting (0: ignore, 1: track)
	
	Calling this function will result in skipping the current keystroke
*/
extern void setKBDEventTrack(uint8_t setting);
