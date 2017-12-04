#include "ir.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

//int irLength = 0, irCapacity = IRLIST_INIT_SIZE;

/*void initIRList() {
	irList = (InterCode*)malloc(IRLIST_INIT_SIZE * sizeof(InterCode));
	if (irList == NULL) {
		printf("initIRList error!\n"); return;
	}
	irLength = 0;
	irCapacity = IRLIST_INIT_SIZE;
}

void insertIRList(InterCode ir) {
	if (irLength >= irCapacity) {
		irList = (InterCode*)realloc(irList, (irCapacity + IRLIST_INIT_SIZE) * sizeof(InterCode));
		irCapacity += IRLIST_INIT_SIZE;
	}

	irList[irLength++] = ir;
}*/

Operand createOperand(int kind, ...) {
	Operand op = (Operand)malloc(sizeof(struct Operand_));
	op->kind = kind;

	va_list args;
	va_start(args, kind);

	switch (kind) {
		case TEMP:
		case VARIABLE:
		case ADDRESS_V: case ADDRESS_T:
		case LABEL:
			op->u.var_no = va_arg(args, int); break;
		case CONSTANT:
			op->u.value = va_arg(args, int); break;
		case FUNCTION:
			op->u.name = va_arg(args, char*); break;
		default: printf("createOperand error!\n"); break;
	}

	va_end(args);
	return op;
}

InterCode createInterCode(int kind, ...) {
	InterCode irCode = (InterCode)malloc(sizeof(struct InterCodes_));
	irCode->code.kind = kind;
	
	va_list args;
	va_start(args, kind);
	
	switch (kind) {
		case IR_LABEL:
		case IR_FUNCTION:
		case IR_GOTO:
		case IR_RETURN:
		case IR_ARG:
		case IR_PARAM:
		case IR_READ:
		case IR_WRITE:
			irCode->code.u.singleOP.op = va_arg(args, Operand); break;
		case IR_ASSIGN:
		case IR_ADDRESS:
		case IR_LOAD:
		case IR_STORE:
		case IR_CALL:
			irCode->code.u.doubleOP.left = va_arg(args, Operand);
			irCode->code.u.doubleOP.right = va_arg(args, Operand);
			break;
		case IR_ADD:
		case IR_SUB:
		case IR_MUL:
		case IR_DIV:
			irCode->code.u.tripleOP.result = va_arg(args, Operand);
			irCode->code.u.tripleOP.op1 = va_arg(args, Operand);
			irCode->code.u.tripleOP.op2 = va_arg(args, Operand);
			break;
		case IR_IF:
			irCode->code.u.ifOP.op1 = va_arg(args, Operand);
			irCode->code.u.ifOP.op2 = va_arg(args, Operand);
			irCode->code.u.ifOP.label = va_arg(args, Operand);
			irCode->code.u.ifOP.relop = va_arg(args, char*);
			break;
		case IR_DEC:
			irCode->code.u.decOP.op = va_arg(args, Operand);
			irCode->code.u.decOP.size = va_arg(args, int);
			break;
		default: printf("createInterCode error!\n"); break;
	}

	return irCode;
}

InterCode concat_code(InterCode code1, InterCode code2) {
	if (code1 == NULL) return code2;
	if (code2 == NULL) return code1;

	InterCode tail = code1;
	while (tail->next != NULL) tail = tail->next;
	tail->next = code2;
	code2->prev = tail;
	return code1;
}

InterCode concat_codes(int argc, ...) {
	va_list args;
	va_start(args, argc);

	InterCode head = NULL, tail = NULL;
	for (int i = 0; i < argc; i++) {
		tail = va_arg(args, InterCode);
		head = concat_code(head, tail);
	}
	va_end(args);
	return head;
}

/* TODO : print console first, file! */
void printOperand(Operand op) {
	switch (op->kind) {
		case TEMP:
			printf("t%d", op->u.var_no); break;
		case VARIABLE:
			printf("v%d", op->u.var_no); break;
		case CONSTANT:
			printf("#%d", op->u.value); break;
		case ADDRESS_V:
			printf("*v%d", op->u.var_no); break;
		case ADDRESS_T:
			printf("*t%d", op->u.var_no); break;
		case LABEL:
			printf("label%d", op->u.var_no); break;
		case FUNCTION:
			printf("%s", op->u.name); break;
		default: printf("printOperand error!\n"); break;
	}
//void printOperand(Operand op, FILE *fp) {
/*	char str[64];
	memset(str, 0, sizeof(str));
	
	switch (op->kind) {
	case VARIABLE, ADDRESS:
		sprintf(str, "%d", op->u.var_no); break;
	case CONSTANT:
		sprintf(str, "#%d", op->u.value); break;
	case NAME:
		sprintf(str, "%s", op->u.name); break;
	}
	fputs(str, fp);*/
}

void printInterCode(InterCode irCode) {
	//	printf("%d\n", irCode->code.kind);
	if (irCode == NULL) return;

	switch(irCode->code.kind) {
		case IR_LABEL:
		case IR_FUNCTION:
		{
			switch (irCode->code.kind) {
				case IR_LABEL:		printf("LABEL ");		break;
				case IR_FUNCTION:	printf("FUNCTION ");	break;
			}
			printOperand(irCode->code.u.singleOP.op);
			printf(" :");
			break;
		}

		case IR_GOTO:
		case IR_RETURN:
		case IR_ARG:
		case IR_PARAM:
		case IR_READ:
		case IR_WRITE:
		{
			switch (irCode->code.kind) {
				case IR_GOTO:		printf("GOTO ");		break;
				case IR_RETURN:		printf("RETURN ");		break;
				case IR_ARG:		printf("ARG ");			break;
				case IR_PARAM:		printf("PARAM ");		break;
				case IR_READ:		printf("READ ");		break;
				case IR_WRITE:		printf("WRITE ");		break;
			}
			printOperand(irCode->code.u.singleOP.op);
			break;
		}

		case IR_ASSIGN:
		case IR_ADDRESS:
		case IR_LOAD:
		case IR_CALL:
		{
			printOperand(irCode->code.u.doubleOP.left);
			switch (irCode->code.kind) {
				case IR_ASSIGN:		printf(" := ");			break;
				case IR_ADDRESS:	printf(" := &");		break;
				case IR_LOAD:		printf(" := *");		break;
				case IR_CALL:		printf(" := CALL ");	break;
			}
			printOperand(irCode->code.u.doubleOP.right);
			break;
		}
		
		case IR_STORE:
			printf("*");
			printOperand(irCode->code.u.doubleOP.left);
			printf(" := ");
			printOperand(irCode->code.u.doubleOP.right);
			break;

		case IR_ADD:
		case IR_SUB:
		case IR_MUL:
		case IR_DIV:
			printOperand(irCode->code.u.tripleOP.result);
			printf(" := ");
			printOperand(irCode->code.u.tripleOP.op1);
			switch (irCode->code.kind) {
				case IR_ADD:		printf(" + ");			break;
				case IR_SUB:		printf(" - ");			break;
				case IR_MUL:		printf(" * ");			break;
				case IR_DIV:		printf(" / ");			break;
			}
			printOperand(irCode->code.u.tripleOP.op2);
			break;

		case IR_IF:
			printf("IF ");
			printOperand(irCode->code.u.ifOP.op1);
			printf(" %s ", irCode->code.u.ifOP.relop);
			printOperand(irCode->code.u.ifOP.op2);
			printf(" GOTO ");
			printOperand(irCode->code.u.ifOP.label);
			break;

		case IR_DEC:
			printf("DEC ");
			printOperand(irCode->code.u.decOP.op);
			printf(" %d", irCode->code.u.decOP.size);
			break;

		default:
			printf("printInterCode error! kind = %d.", irCode->code.kind);
			break;
	}

	printf("\n");
}		

/* TODO : print console first, file! */
void printInterCodes(InterCode irList) {
	while (irList != NULL) {
		printInterCode(irList);
		irList = irList->next;
	}
//void printInterCode(char *filename) {
/*	FILE *fp;

	if (strcmp(filename, "stdout") == 0) fp = stdout;
	else fp = fopen(filename, "r");

	if (fp == NULL) {
		printf("printInterCode open file error!\n");
		return;
	}

	for (int i = 0; i < irLength; i++) {
		InterCode ir = irList[i];
		if (ir == NULL) continue;

		switch (ir->kind) {
		case IR_LABEL, IR_FUNCTION:
			switch (ir->kind) {
			case IR_LABEL:		fputs("LABEL ", fp);	break;
			case IR_FUNCTION:	fputs("FUNCTION ", fp);	break;
			}
			printOP(ir->u.singleOP.op, fp);
			fputs(" : ", fp);
			break;

		case IR_GOTO, IR_RETURN, IR_ARG, IR_PARAM, IR_READ, IR_WRITE: 
			switch (ir->kind) {
			case IR_GOTO:		fputs("GOTO ", fp);		break;
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
	fclose(fp);*/
}
