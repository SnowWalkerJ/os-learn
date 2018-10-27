#include <stddef.h>
#include "memory.h"
#include <libs/stdlib.h>

#define START_ADDR 0x10000
#define MAX_SIZE 0x80000000

typedef struct{
	uint32_t start;
	uint32_t size;
	char used;
}__attribute__((packed)) MemoryBlock;

#define MB_SIZE sizeof(MemoryBlock)
#define MEM_TABLE_START ((MemoryBlock*)0x0f000)

static MemoryBlock *memTable = MEM_TABLE_START;
static uint32_t numBlocks = 1;

void recycleMemBlock(uint32_t i){
	(memTable+i)->used = 0;
	if (i > 0 && !(memTable+i-1)->used) {
		(memTable+i-1)->size += (memTable+i)->size;
		i--;
		memcpy(memTable+(i+2)*MB_SIZE, memTable+(i+1)*MB_SIZE, numBlocks-i-1);
		numBlocks--;
	}
	if (!(memTable+i+1)->used) {
		(memTable+i)->size += (memTable+i+1)->size;
		memcpy(memTable+(i+2)*MB_SIZE, memTable+(i+1)*MB_SIZE, numBlocks-i-1);
		numBlocks--;
	}
}

void initMemTable() {
	memTable->start = START_ADDR;
	memTable->size = MAX_SIZE;
	memTable->used = 0;
	numBlocks = 1;
}

void* malloc(size_t size) {
	size_t i = 0;
	while(i < (START_ADDR - (int)MEM_TABLE_START) / MB_SIZE) {
		MemoryBlock *mb = memTable+i;
		if (mb->used) i++;
		else if (mb->size >= size) {
			memcpy(mb, (mb+1), MB_SIZE*(numBlocks-i+1));
			numBlocks++;
			(memTable+i+1)->start = mb->start + size;
			(memTable+i+1)->size = mb->size - size;
			mb->size = size;
			mb->used = 1;
			return (void*)mb->start;
		}
	}
	return 0;
}

int free(void* ptr) {
	uint32_t i;
	for (i = 0; i < numBlocks; i++){
		if ((memTable+i)->start == (uint32_t)ptr) {
			recycleMemBlock(i);
			return 0;
		}
	}
	return -1;
}

