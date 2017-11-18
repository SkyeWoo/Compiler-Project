#ifndef _SEMANTIC_H_
#define _SEMANTIC_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node.h"

typedef struct Type_* Type;
typedef struct FieldList_* FieldList;

typedef enum { false, true } bool;

typedef struct Type_ {
	enum { BASIC, ARRAY, STRUCTURE, FUNCTION } kind;

	union {
		// basic type(INT or FLOAT)
		struct {
			int type; char* name;
		} basic;

		// array type
		struct {
			Type elem; int size;
		} array;

		// structure type itself is a list
		FieldList structure;

		// function type
		struct {
			bool defined;
			Type rtnType;
			FieldList params;
			int paramNum;
		} function;
	} u;
} Type_;

typedef struct FieldList_ {
	char* name;
	int lineno;
	Type type;
	FieldList tail;
} FieldList_;

unsigned int hash_pjw(char* name);

void initTable();
bool insertSymbol(FieldList f, FieldList* table);
FieldList searchSymbol(char* name, FieldList* table);

bool TypeEqual(Type type1, Type type2);

void check_declared_undefined();

void handle_Program(Node* root);
void handle_ExtDefList(Node* root);
void handle_ExtDef(Node* root);
void handle_ExtDecList(Node* root, Type basic);
Type handle_Specifier(Node* root);
FieldList handle_VarDec(Node* root, Type basic);
void handle_FunDec(Node* root, Node* ExtDef, Type basic, bool defined);
FieldList handle_ParamDec(Node* root, Node* ExtDef);
void handle_CompSt(Node* root, Type rtnType);
void handle_StmtList(Node* root, Type rtnType);
void handle_Stmt(Node* root, Type rtnType);
void handle_DefList(Node* root);
void handle_Def(Node* root);
void handle_DecList(Node* root, Type basic);
void handle_Dec(Node* root, Node* DecList, Type basic);
Type handle_Exp(Node* root);
void handle_Args(Node* root, Type* type);

#define INT 1
#define FLOAT 2

#define HASH_TABLE_SIZE 16384
FieldList variable_table[HASH_TABLE_SIZE];
FieldList function_table[HASH_TABLE_SIZE];

#endif
