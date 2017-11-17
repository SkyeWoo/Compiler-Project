#include "semantic.h"

unsigned int hash_pjw(char* name) {
	unsigned int val = 0, i;
	for (; *name; ++name) {
		val = (val << 2) + *name;
		if (i = val & ~0x3fff) val = (val ^ (i >> 12)) & 0x3fff;
	}
	return val;
}

#define HASH_TABLE_SIZE 16384
FieldList hashTable[HASH_TABLE_SIZE];

void initHashTable() {
	for (int i = 0; i < HASH_TABLE_SIZE; i++)
		hashTable[i] = NULL;
}
