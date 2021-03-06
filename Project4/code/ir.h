#ifndef __IR_H__
#define __IR_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "node.h"

#define IRLIST_INIT_SIZE 10

typedef struct Operand_* Operand;
typedef struct InterCodes_* InterCode;

struct Operand_ {
	// t1, t2, ... - TEMP
	// v1, v2, ... - VARIABLE
	// #1, ... - CONSTANT
	// label1, ... - LABEL
	// main, ... - FUNCTION
	// &v1, &t1, ... - ADDRESS
	// *t1, ... - VALUE
	enum { TEMP, VARIABLE, CONSTANT, LABEL, FUNCTION, ADDRESS, ADDRESS_V, VALUE } kind;

	union {
		int var_no;		// TEMP, VARIABLE, LABEL
		int value;		// CONSTANT
		char* name;		// FUNCTION
		struct { Operand op; int size; } t;
						// ADDRESS, ADDRESS_V
		Operand op;		// VALUE
	} u;
};

struct InterCode_ {
	enum {
		IR_LABEL, IR_FUNCTION, IR_ASSIGN, 
		IR_ADD, IR_SUB, IR_MUL, IR_DIV,
		IR_ADDRESS, IR_LOAD, IR_STORE, 
		IR_GOTO, IR_IF, IR_RETURN, IR_DEC,
		IR_ARG, IR_CALL, IR_PARAM,
		IR_READ, IR_WRITE
	} kind;

	union {
		// IR_LABEL, IR_FUNCTION, IR_GOTO, IR_RETURN, IR_ARG, IR_PARAM, IR_READ, IR_WRITE
		struct { Operand op; } singleOP;

		// IR_ASSIGN, IR_ADDRESS, IR_LOAD, IR_STORE, IR_CALL
		struct { Operand left, right; } doubleOP;

		// IR_ADD, IR_SUB, IR_MUL, IR_DIV
		struct { Operand result, op1, op2; } tripleOP;

		// IR_IF
		struct { Operand op1, op2, label; char* relop; } ifOP;

		// IR_DEC
		struct { Operand op; int size; } decOP;
	} u;
};

struct InterCodes_ {
	struct InterCode_ code;
	struct InterCodes_ *prev, *next;
};

Operand createOperand(int kind, ...);
InterCode createInterCode(int kind, ...);
InterCode concat_code(InterCode code1, InterCode code2);
InterCode concat_codes(int argc, ...);
void printOperand(Operand op, FILE* fp);
void printInterCode(InterCode irCode, FILE* fp);
void printInterCodes(InterCode irList);

InterCode translate_Program(Node* root);
InterCode translate_ExtDefList(Node* root);
InterCode translate_ExtDef(Node* root);
int translate_VarDec(Node* root, Operand* op);
InterCode translate_FunDec(Node* root);
InterCode translate_VarList(Node* root);
InterCode translate_ParamDec(Node* root);
InterCode translate_CompSt(Node* root);
InterCode translate_StmtList(Node* root);
InterCode translate_Stmt(Node* root);
InterCode translate_Cond(Node* root, Operand label_true, Operand label_false);
InterCode translate_DefList(Node* root);
InterCode translate_Def(Node* root);
InterCode translate_DecList(Node* root);
InterCode translate_Dec(Node* root);
InterCode translate_Exp(Node* root, Operand* op);
InterCode translate_Args(Node* root, Operand* arg_list, int i);

extern int temp_no, var_no, label_no; 
extern char* filename;

#endif
