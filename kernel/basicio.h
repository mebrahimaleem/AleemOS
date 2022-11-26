//basicio.h
//
//IO libraries for kernel using VGA

extern void clearVGA(void);

extern void put(volatile const char str, volatile uint8_t x, volatile uint8_t y, volatile uint8_t color);

extern void vgaprint(volatile char* volatile str, volatile uint8_t col);

extern void vgaprintint(uint32_t num, uint8_t base, uint8_t col);
