//kbd.h
//
//provides declarations for the keyboard driver

typedef struct KBDEventQueue {
	uint16_t keyCode;
	volatile struct KBDEventQueue* volatile next;
	volatile struct KBDEventQueue* volatile prev;
} KBDEventQueue;

extern void KBDResetMods(void);

extern volatile KBDEventQueue* volatile KBDNextEvent;

extern void ISR21_handler(uint32_t byte);

extern char toAscii(uint32_t keycode);

extern void setKBDEventTrack(uint8_t setting);
