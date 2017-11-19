#include "semantic.h"

unsigned int hash_pjw(char* name) {
	unsigned int val = 0, i;
	for (; *name; ++name) {
		val = (val << 2) + *name;
		if (i = val & ~0x3fff) val = (val ^ (i >> 12)) & 0x3fff;
	}
	return val;
}

void initTable() {
	for (int i = 0; i < HASH_TABLE_SIZE; i++) {
		variable_table[HASH_TABLE_SIZE] = NULL;
		function_table[HASH_TABLE_SIZE] = NULL;
	}
}

bool insertSymbol(SymbolList s, SymbolList* table) {
	if (s == NULL || s->field == NULL || s->field->name == NULL) return false;

	unsigned int key = hash_pjw(s->field->name);

	// no collision
	if (table[key] == NULL) {
		table[key] = s; return true;
	}

	// collision
	while (1) {
		key = (++key) % HASH_TABLE_SIZE;
		SymbolList q = table[key];
		if (q == NULL) {
			table[key] = s; return true;
		}
	}

	return false;
}

SymbolList searchSymbol(char* name, SymbolList* table) {
	if (name == NULL) return NULL;

	unsigned int key = hash_pjw(name);

	SymbolList p = table[key];
	while (p != NULL && p->field != NULL) {
		if (strcmp(name, p->field->name) == 0) return p;
		key = (++key) % HASH_TABLE_SIZE;
		p = table[key];
	}

	return NULL;
}
