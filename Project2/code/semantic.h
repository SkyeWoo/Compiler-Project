#ifndef _SEMANTIC_H_
#define _SEMANTIC_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node.h"

typedef struct VarType_* VarType;
typedef struct FuncType_* FuncType;
typedef struct FieldList_* FieldList;
typedef struct SymbolList_* SymbolList;

typedef enum { false, true } bool;

typedef struct VarType_ {
	enum { BASIC, ARRAY, STRUCTURE } kind;

	union {
		// basic type(INT or FLOAT)
		struct {
			int type; char* name;
		} basic;

		// array type
		struct {
			VarType elem; int size;
		} array;

		// structure type itself is a list
		FieldList structure;
	} u;
} VarType_;

typedef struct FuncType_ {
	bool defined;
	VarType rtnType;
	FieldList params;
	int paramNum;
} FuncType_;

typedef struct FieldList_ {
	char* name;
	union {
		VarType variable;

		FuncType_* function;
	} type;

	FieldList tail;
} FieldList_;

typedef struct SymbolList_ {
	int lineno;
	FieldList field;
} SymbolList_;

unsigned int hash_pjw(char* name);

void initTable();
bool insertSymbol(SymbolList f, SymbolList* table);
SymbolList searchSymbol(char* name, SymbolList* table);

bool VarTypeEqual(VarType type1, VarType type2);
bool FuncTypeEqual(FuncType type1, FuncType type2);

void check_declared_undefined();

void handle_Program(Node* root);
void handle_ExtDefList(Node* root);
void handle_ExtDef(Node* root);
void handle_ExtDecList(Node* root, VarType basic);
VarType handle_Specifier(Node* root);
SymbolList handle_VarDec(Node* root, VarType basic);
void handle_FunDec(Node* root, Node* ExtDef, VarType basic, bool defined);
SymbolList handle_ParamDec(Node* root, Node* ExtDef);
void handle_CompSt(Node* root, VarType rtnType);
void handle_StmtList(Node* root, VarType rtnType);
void handle_Stmt(Node* root, VarType rtnType);
void handle_DefList(Node* root);
void handle_Def(Node* root);
void handle_DecList(Node* root, VarType basic);
void handle_Dec(Node* root, Node* DecList, VarType basic);
VarType handle_Exp(Node* root);
void handle_Args(Node* root, FuncType* type);

#define INT 1
#define FLOAT 2

#define HASH_TABLE_SIZE 16384
SymbolList variable_table[HASH_TABLE_SIZE];
SymbolList function_table[HASH_TABLE_SIZE];

#endif
