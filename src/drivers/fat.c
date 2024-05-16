//fat.c

#include <stdint.h>
#include <kernel.h>
#include <utils.h>
#include <memory.h>
#include <paging.h>
#include <fat.h>

#define FAT_BASE (uint8_t*)0x7e00
#define ROOT_DIR (uint8_t*)0x9000

#define LOGICAL_TO_VIRTUAL(L) ((31 + L) * 512 + dataAreaBase)


uint8_t* dataAreaBase;
FileEntry rd;

//TODO: sync ram fs with disk fs

const char* fat_ls(const char* path) {
	char* ret;
	FileList* files;
	if (strcmp((uint8_t*)path, (uint8_t*)"/") == 1) { //check if path is root directory
		files = _fat_iterFiles(rd);
		ret = malloc(12 * files->count + 1);
		uint32_t ci = 0;
		for (FileList* i = files; i != 0; i = i->n) {
			for (uint8_t j = 0; j < 11; j++) ret[ci + j] = (char)i->f.name[j];
			ret[ci + 11] = '\n';
			ci += 12;
		}
		ret[12 * files->count] = 0;
		return ret;
	}
	return 0; //TODO: implemented subdirectories
}

uint32_t fat_init(uint32_t avAddr) {
	mapMemory4M(kernelPD, FS_DATA_BASE, FS_DATA_BASE, 3);
	dataAreaBase = (uint8_t* )FS_DATA_BASE;
	rd.name[0] = '/';
	rd.name[1] =
		rd.name[2] =
		rd.name[3] =
		rd.name[4] =
		rd.name[5] =
		rd.name[6] =
		rd.name[7] =
		rd.name[8] =
		rd.name[9] =
		rd.name[10] = ' '; //NOTE: this intentionally overflows to clear ex
	rd.cluster = -12; //we need to go backwards from data area to get to root directory
	rd.size = 1572864;

	//copy fat
	for (uint32_t i = 0; i < 9 * 512; i++) {
		*(dataAreaBase + 1 * 512 + i) = //FAT1
			*(dataAreaBase + 10 * 512 + i) = *(uint8_t* )(0x7e00 + i); //FAT2
	}

	//copy root directory
	for (uint32_t i = 0; i < 14 * 512; i++) {
		*(dataAreaBase + 19 * 512 + i) = *(uint8_t* )(0x9000 + i);
	}

	return avAddr;
}

FileList* _fat_iterFiles(FileEntry dir) {
	FileList* ret = (FileList*)malloc(sizeof(FileList));
	ret->f.name[0] = '.';
	ret->f.name[1] =
		ret->f.name[2] =
		ret->f.name[3] =
		ret->f.name[4] =
		ret->f.name[5] =
		ret->f.name[6] =
		ret->f.name[7] =
		ret->f.name[8] =
		ret->f.name[9] =
		ret->f.name[10] = ' '; //NOTE: this intentionally overflows to clear ex

	ret->n = (FileList*)malloc(sizeof(FileList));
	FileList* n = ret->n;
	

	n->f.name[0] = '.';
	n->f.name[1] = '.';
	n->f.name[2] =
		n->f.name[3] =
		n->f.name[4] =
		n->f.name[5] =
		n->f.name[6] =
		n->f.name[7] =
		n->f.name[8] =
		n->f.name[9] =
		n->f.name[10] = ' ';
	n->n = 0;

	uint32_t count = 2;

	const FileEntry* files = (FileEntry* )LOGICAL_TO_VIRTUAL(dir.cluster);
	const uint32_t dirLim = dir.size / 32;
	for (uint32_t i = 0; i < dirLim; i++) {
		if (files[i].name[0] == 0xe5) continue; //free
		if (files[i].name[0] == 0x00) break; //remaining is free
		n->n = (FileList*)malloc(sizeof(FileList));
		n = n->n;
		n->f = files[i];
		n->n = 0;
		count++;
	}

	ret->count = count;
	return ret;
}
