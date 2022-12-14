//utils.h
//
//provides declarations for generic utilities in AleemOS

//Converts a int32_t to string
extern uint8_t* int32_to_string(int32_t num, uint8_t base);

//Converts a uint32_t to string
extern uint8_t* uint32_to_string(uint32_t num, uint8_t base);

//Sleeps the process time ms
extern void sleepms(uint32_t time);

//Compares two string
extern uint8_t strcmp(uint8_t* str1, uint8_t* str2);

//Gets the lenght of a string
extern uint32_t strlen(uint8_t* str);
