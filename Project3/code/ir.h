#ifndef __IR_H__
#define __IR_H__

#include <stdio.h>

#define IRLIST_INIT_SIZE 10

typedef struct Operand_* Operand;
typedef struct InterCode_* InterCode;

struct Operand_ {
	// NAME for label/function name
	enum { VARIABLE, CONSTANT, ADDRESS, NAME } kind;

	union {
		int var_no;	// VARIABLE, ADDRESS
		int value;	// CONSTANT
		char *name;	// NAME
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
		struct { Operand op1, op2, label; char relop[32]; } ifOP;

		// IR_DEC
		struct { Operand op; int size; } decOP;
	} u;
};

struct InterCodes {
	InterCode_ code;
	struct InterCodes *prev, *next;
};

void initIRList();
void insertIRList(InterCode ir);
void printOP(Operand op, FILE *fp);
void printInterCode(char *filename);

#endif
