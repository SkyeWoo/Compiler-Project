#include "ir.h"
#include <stdlib.h>
#include <string.h>

InterCode *IRList;
//int IRlength = 0, IRcapacity = IRLIST_INIT_SIZE;
int IRlength, IRcapacity;

void initIRList() {
	IRList = (InterCode*)malloc(IRLIST_INIT_SIZE * sizeof(InterCode));
	if (IRList == NULL) {
		printf("initIRList error!\n"); return;
	}
	IRlength = 0;
	IRcapacity = IRLIST_INIT_SIZE;
}

void insertIRList(InterCode ir) {
	if (IRlength >= IRcapacity) {
		IRList = (InterCode*)realloc(IRList, (IRcapacity + IRLIST_INIT_SIZE) * sizeof(InterCode));
		IRcapacity += IRLIST_INIT_SIZE;
	}

	IRList[IRlength++] = ir;
}

void printOP(Operand op, FILE *fp) {
	char str[64];
	memset(str, 0, sizeof(str));
	
	switch (op->kind) {
	case VARIABLE, ADDRESS:
		sprintf(str, "%d", op->u.var_no); break;
	case CONSTANT:
		sprintf(str, "#%d", op->u.value); break;
	case NAME:
		sprintf(str, "%s", op->u.name); break;
	}
	fputs(str, fp);
}

void printInterCode(char *filename) {
	FILE *fp;

	if (strcmp(filename, "stdout") == 0) fp = stdout;
	else fp = fopen(filename, "r");

	if (fp == NULL) {
		printf("printInterCode open file error!\n");
		return;
	}

	for (int i = 0; i < IRlength; i++) {
		InterCode ir = IRList[i];
		if (ir == NULL) continue;

		switch (ir->kind) {
		case IR_LABEL, IR_FUNCTION, IR_GOTO:
			switch (ir->kind) {
			case IR_LABEL:		fputs("LABEL ", fp);	break;
			case IR_FUNCTION:	fputs("FUNCTION ", fp);	break;
			case IR_GOTO:		fputs("GOTO ", fp);		break;
			}
			printOP(ir->u.singleOP.op, fp);
			fputs(" : ", fp);
			break;

		case IR_RETURN, IR_ARG, IR_PARAM, IR_READ, IR_WRITE: 
			switch (ir->kind) {
			case IR_RETURN:		fputs("RETURN ", fp);	break;
			case IR_ARG:		fputs("ARG ", fp);		break;
			case IR_PARAM:		fputs("PARAM ", fp);	break;
			case IR_READ:		fputs("READ ", fp);		break;
			case IR_WRITE:		fputs("WRITE ", fp);	break;
			}
			printOP(ir->u.singleOP.op, fp);
			break;

		case IR_ASSIGN, IR_ADDRESS, IR_LOAD, IR_CALL:
			printOP(ir->u.doubleOP.left, fp);
			switch (ir->kind) {
			case IR_ASSIGN:		fputs(" := ", fp);		break;
			case IR_ADDRESS:	fputs(" := &", fp);		break;
			case IR_LOAD:		fputs(" := *", fp);		break;
			case IR_CALL:		fputs(" := CALL ", fp);	break;
			}
			printOP(ir->u.doubleOP.right, fp);
			break;

		case IR_STORE:
			fputs("*", fp);
			printOP(ir->u.doubleOP.left, fp);
			fputs(" := ", fp);
			printOP(ir->u.doubleOP.right, fp);
			break;
		
		case IR_ADD, IR_SUB, IR_MUL, IR_DIV:
			printOP(ir->u.tripleOP.result, fp);
			fputs(" := ", fp);
			printOP(ir->u.tripleOP.op1, fp);
			switch (ir->kind) {
			case IR_ADD:		fputs(" + ", fp);		break;
			case IR_SUB:		fputs(" - ", fp);		break;
			case IR_MUL:		fputs(" * ", fp);		break;
			case IR_DIV:		fputs(" / ", fp);		break;
			}
			printOP(ir->u.tripleOP.op2, fp);
			break;
		
		case IR_IF:
			fputs("IF ", fp);
			printOP(ir->u.ifOP.op1, fp);
			fputs(" ", fp);
			fputs(ir->u.ifOP.relop, fp);
			fputs(" ", fp);
			printOP(ir->u.ifOP.op2, fp);
			fputs(" GOTO ", fp);
			printOP(ir->u.ifOP.label, fp);
			break;

		case IR_DEC:
			fputs("DEC ", fp);
			printOP(ir->u.decOP.op, fp);
			char str[10];
			memset(str, 0, sizeof(str));
			sprintf(str, " %d ", ir->u.decOP.size);
			fputs(str, fp);
			break;

		default: break;
		}

		fputs("\n", fp);
	}
	fclose(fp);
}
