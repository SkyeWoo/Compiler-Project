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
		variable_table[i] = NULL;
		function_table[i] = NULL;
	}
}

bool insertSymbol(SymbolList s, SymbolList* table) {
	if (s == NULL || s->field == NULL || s->field->name == NULL) return false;

	unsigned int key = hash_pjw(s->field->name);

	// no collision
	if (table[key] == NULL) {
		table[key] = s;
		table[key]->dead = false;
		table[key]->next = NULL;
		return true;
	}

	s->next = table[key];
	table[key] = s;
	table[key]->dead = false;
	return true;
}

SymbolList searchSymbol(char* name, SymbolList* table) {
	if (name == NULL) return NULL;

	unsigned int key = hash_pjw(name);

	SymbolList p = table[key];
	while (p != NULL) {
		if (strcmp(name, p->field->name) == 0 && p->dead == false)
			return p;
		p = p->next;
	}

	return NULL;
}

Operand getOP(SymbolList symbol) {
	return symbol->field->op;
}
