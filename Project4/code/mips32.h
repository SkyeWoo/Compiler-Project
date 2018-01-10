#ifndef __MIPS32_H__
#define __MIPS32_H__

#include <string.h>
#include "semantic.h"
#include "ir.h"

#define REG_NUM 32

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

void printMIPS(InterCode irList);
int getReg(Operand op, FILE* fp);

extern char* filename;

#endif
