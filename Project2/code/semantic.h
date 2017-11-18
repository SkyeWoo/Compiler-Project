#ifndef _SEMANTIC_H_
#define _SEMANTIC_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node.h"

typedef enum { false, true } bool;

typedef struct Type_ *Type;
typedef struct FieldList_ *FieldList;

#define INT 1
#define FLOAT 2

struct Type_ {
	enum { BASIC, ARRAY, STRUCTURE, FUNCTION } kind;

	union {
		// basic type(INT or FLOAT)
		int basic;

		// array type
		struct {
			Type elem; int size;
		} array;

		// structure type itself is a list
		FieldList structure;

		// function type
		struct {
			Type rtnType;
			FieldList params;
			int paramNum;
		} function;
	} u;
};

struct FieldList_ {
	char* name;
	Type type;
	FieldList tail;
};

unsigned int hash_pjw(char* name);

void initTable();
bool insertSymbol(FieldList f, FieldList* table);
FieldList searchSymbol(char* name, FieldList* table);

void handle_Program(Node* root);
void handle_ExtDefList(Node* root);

#endif
