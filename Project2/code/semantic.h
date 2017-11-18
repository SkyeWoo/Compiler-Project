#ifndef _SEMANTIC_H_
#define _SEMANTIC_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node.h"

typedef enum { false, true } bool;

typedef struct Type_* Type;
typedef struct FieldList_* FieldList;

#define INT 1
#define FLOAT 2

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
			Type rtnType;
			FieldList params;
			int paramNum;
		} function;
	} u;
} Type_;

typedef struct FieldList_ {
	char* name;
	Type type;
	FieldList tail;
} FieldList_;

unsigned int hash_pjw(char* name);

void initTable();
bool insertSymbol(FieldList f, FieldList* table);
FieldList searchSymbol(char* name, FieldList* table);

void handle_Program(Node* root);
void handle_ExtDefList(Node* root);
void handle_ExtDef(Node* root);
void handle_ExtDecList(Node* root, Type basic);
Type handle_Specifier(Node* root);
FieldList handle_VarDec(Node* root, Type basic);
void handle_FunDec(Node* root, Node* ExtDef, Type basic);
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

#endif
