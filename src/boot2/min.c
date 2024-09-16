//min.c

#include <stdint.h>
#include <min.h>

uint8_t strcmp(uint8_t* volatile f, uint8_t* volatile s) {
	for (uint32_t i = 0; i < 11; i++) {
		if (f[i] != s[i]) return 0;
	}
	return 1;
}

void min(void) {
	uint32_t ci = *(uint32_t* volatile)(BPB + 44)  - 2; //cluster index
	uint32_t clsz = (uint32_t)*(uint16_t* volatile)(BPB + 11) * (uint32_t)*(uint8_t* volatile)(BPB + 13); //cluster size in bytes
	uint8_t fs = 0;

	uint32_t* FAT = (uint32_t* volatile)FAT_BASE;

	uint32_t sc; //source cluster
	uint32_t offset = 0;
	while (1) {
		for (uint32_t j = 0; j < clsz; j += 32) {
			if (strcmp((uint8_t* volatile)(DATA_BASE + ci * clsz + j), (uint8_t*)"KERNEL  BIN")) {
				sc = (uint32_t)*(uint16_t* volatile)(DATA_BASE + ci * clsz + j + 26) + ((uint32_t)*(uint16_t* volatile)(DATA_BASE + ci * clsz + j + 20) << 16) - 2;
				while (1) {
					for (uint32_t i = 0; i < clsz; i++) // Copy cluster
						*(uint32_t* volatile)(KERNEL_BASE + offset + i) = *(uint32_t* volatile)(DATA_BASE + sc * clsz + j + i);
					offset += clsz;
					if (FAT[sc+2] >= 0x0FFFFFFF) break; //EOF
					else sc = FAT[sc+2] - 2;
				}
				fs++;
			}
			else if (strcmp((uint8_t* volatile)(DATA_BASE + ci * clsz + j), (uint8_t*)"SH      ELF")) {
				sc = (uint32_t)*(uint16_t* volatile)(DATA_BASE + ci * clsz + j + 26) + ((uint32_t)*(uint16_t* volatile)(DATA_BASE + ci * clsz + j + 20) << 16) - 2;
				while (1) {
					for (uint32_t i = 0; i < clsz; i++) // Copy cluster
						*(uint32_t* volatile)(DEFAPP_BASE + offset + i) = *(uint32_t* volatile)(DATA_BASE + sc * clsz + j + i);
					offset += clsz;
					if (FAT[sc+2] >= 0x0FFFFFFF) break; //EOF
					else sc = FAT[sc+2] - 2;
				}
				fs++;
			}
		}
		if (fs == 2) break;
		else if (FAT[ci+2] >= 0x0FFFFFFF) {
			//file not found
			////TODO: implement missing file error
			while (1) ;
		}
		else ci = FAT[ci+2] - 2;
	}
	return;
}
