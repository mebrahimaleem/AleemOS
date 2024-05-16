//fat.h

typedef struct {
	uint8_t name [11]; //last 3 bits are extension
	uint8_t attr;
	uint16_t res;
	uint16_t create_tm;
	uint16_t create_dt;
	uint16_t access_dt;
	uint16_t ign;
	uint16_t write_tm;
	uint16_t write_dt;
	int16_t cluster;
	uint32_t size;

} __attribute((packed)) FileEntry;

typedef struct FileList {
	FileEntry f;
	struct FileList* n;
	uint32_t count;
} FileList;

extern const char* fat_ls(const char* path);

extern uint32_t fat_init(uint32_t avAddr);

extern FileList* _fat_iterFiles(FileEntry dir);
