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
FieldList variable_table[HASH_TABLE_SIZE];
FieldList structure_table[HASH_TABLE_SIZE];
FieldList function_table[HASH_TABLE_SIZE];

void initTable() {
	for (int i = 0; i < HASH_TABLE_SIZE; i++) {
		variable_table[HASH_TABLE_SIZE] = NULL;
		structure_table[HASH_TABLE_SIZE] = NULL;
		function_table[HASH_TABLE_SIZE] = NULL;
		//hashTable[i] = NULL;
	}
}

bool insertSymbol(FieldList f, FieldList* table) {
	if (f == NULL || f->name == NULL) return false;

	unsigned int key = hash_pjw(f->name);

	// no collision
	if (table[key] == NULL) {
		table[key] = f; return true;
	}

	// collided
	while (1) {
		key = (++key) % HASH_TABLE_SIZE;
		FieldList q = table[key];
		if (q == NULL) {
			table[key] = f; return true;
		}
	}

	return false;
}

FieldList searchSymbol(char* name, FieldList* table) {
	if (name == NULL) return NULL;

	unsigned int key = hash_pjw(name);

	FieldList p = table[key];
	while (p != NULL) {
		if (strcmp(name, p->name) == 0) return p;
		key = (++key) % HASH_TABLE_SIZE;
		p = table[key];
	}

	return NULL;
}

void handle_Program(Node* root) {
	if (root == NULL) return;

	// Program -> ExtDefList
	if (root->childsum != 0)
		handle_ExtDefList(root->child[0]);
}

void handle_ExtDefList(Node* root) {

}
