#ifndef __MIPS32_H__
#define __MIPS32_H__

#include <string.h>
#include "semantic.h"
#include "ir.h"

#define REG_NUM 32
#define STACKSIZE 1024

typedef struct Var {
	int reg_no;
	Operand op;
	struct Var *next;
} VarDescripter;

typedef struct Reg {
	char name[6];
	int old;
	struct Var *var;
} RegDescripter;

typedef struct {
	int length;

	struct {
		int old;
		VarDescripter* var;
	} stack[STACKSIZE];
} StkDescripter;

void printMIPS(InterCode irList);
int getReg(Operand op, FILE* fp);

extern char* filename;

#endif
