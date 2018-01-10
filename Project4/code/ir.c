#include "ir.h"
#include "semantic.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

Operand createOperand(int kind, ...) {
	Operand op = (Operand)malloc(sizeof(struct Operand_));
	op->kind = kind;

	va_list args;
	va_start(args, kind);

	switch (kind) {
		case TEMP:
		case VARIABLE:
		case LABEL:
			op->u.var_no = va_arg(args, int); break;
		case CONSTANT:
			op->u.value = va_arg(args, int); break;
		case FUNCTION:
			op->u.name = va_arg(args, char*); break;
		case ADDRESS:
		case ADDRESS_V:
			op->u.t.op = va_arg(args, Operand);
			op->u.t.size = va_arg(args, int); break;
		case VALUE:
			op->u.op = va_arg(args, Operand); break;
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

char* filename;

void printOperand(Operand op, FILE* fp) {
	//	printf("op = %d\n", op->kind);
	char str[64];
	memset(str, 0, sizeof(str));
	
	switch (op->kind) {
	case TEMP:
		sprintf(str, "t%d", op->u.var_no); break;
	case VARIABLE:
		sprintf(str, "v%d", op->u.var_no); break;
	case CONSTANT:
		sprintf(str, "#%d", op->u.value); break;
	case LABEL:
		sprintf(str, "label%d", op->u.var_no); break;
	case FUNCTION:
		sprintf(str, "%s", op->u.name); break;
	case ADDRESS:
		switch (op->u.t.op->kind) {
			case TEMP: sprintf(str, "&t%d", op->u.t.op->u.var_no); break;
			case VARIABLE: sprintf(str, "&v%d", op->u.t.op->u.var_no); break;
		}
		break;
	case ADDRESS_V:
		switch (op->u.t.op->kind) {
			case TEMP: sprintf(str, "t%d", op->u.t.op->u.var_no); break;
			case VARIABLE: sprintf(str, "v%d", op->u.t.op->u.var_no); break;
		}
		break;
	case VALUE:
		switch (op->u.op->kind) {
			case TEMP: sprintf(str, "*t%d", op->u.op->u.var_no); break;
			case VARIABLE: sprintf(str, "*v%d", op->u.op->u.var_no); break;
		}
		break;
	default:
		printf("printOperand error!\n"); return;
	}
	fputs(str, fp);
}

void printInterCode(InterCode irCode, FILE* fp) {
//		printf("ir = %d\n", irCode->code.kind);

	if (irCode == NULL) return;

	switch(irCode->code.kind) {
		case IR_LABEL:
		case IR_FUNCTION:
		{
			switch (irCode->code.kind) {
				case IR_LABEL:		fputs("LABEL ", fp);		break;
				case IR_FUNCTION:	fputs("FUNCTION ", fp);		break;
			}
			printOperand(irCode->code.u.singleOP.op, fp);
			fputs(" :", fp);
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
				case IR_GOTO:		fputs("GOTO ", fp);			break;
				case IR_RETURN:		fputs("RETURN ", fp);		break;
				case IR_ARG:		fputs("ARG ", fp);			break;
				case IR_PARAM:		fputs("PARAM ", fp);		break;
				case IR_READ:		fputs("READ ", fp);			break;
				case IR_WRITE:		fputs("WRITE ", fp);		break;
			}
			printOperand(irCode->code.u.singleOP.op, fp);
			break;
		}

		case IR_ASSIGN: 
		case IR_ADDRESS:
		case IR_LOAD:
		case IR_CALL:
		{
			printOperand(irCode->code.u.doubleOP.left, fp);
			switch (irCode->code.kind) {
				case IR_ASSIGN:		fputs(" := ", fp);			break;
				case IR_ADDRESS:	fputs(" := &", fp);			break;
				case IR_LOAD:		fputs(" := *", fp);			break;
				case IR_CALL:		fputs(" := CALL ", fp);		break;
			}
			printOperand(irCode->code.u.doubleOP.right, fp);
			break;
		}
		
		case IR_STORE:
			fputs("*", fp);
			printOperand(irCode->code.u.doubleOP.left, fp);
			fputs(" := ", fp);
			printOperand(irCode->code.u.doubleOP.right, fp);
			break;

		case IR_ADD:
		case IR_SUB:
		case IR_MUL:
		case IR_DIV:
			printOperand(irCode->code.u.tripleOP.result, fp);
			fputs(" := ", fp);
			printOperand(irCode->code.u.tripleOP.op1, fp);
			switch (irCode->code.kind) {
				case IR_ADD:		fputs(" + ", fp);			break;
				case IR_SUB:		fputs(" - ", fp);			break;
				case IR_MUL:		fputs(" * ", fp);			break;
				case IR_DIV:		fputs(" / ", fp);			break;
			}
			printOperand(irCode->code.u.tripleOP.op2, fp);
			break;

		case IR_IF:
			fputs("IF ", fp);
			printOperand(irCode->code.u.ifOP.op1, fp);
			fprintf(fp, " %s ", irCode->code.u.ifOP.relop);
			printOperand(irCode->code.u.ifOP.op2, fp);
			fputs(" GOTO ", fp);
			printOperand(irCode->code.u.ifOP.label, fp);
			break;

		case IR_DEC:
			fputs("DEC ", fp);
			printOperand(irCode->code.u.decOP.op, fp);
			fprintf(fp, " %d", irCode->code.u.decOP.size);
			break;

		default:
			printf("printInterCode error! kind = %d.", irCode->code.kind);
			return;
	}

	fputs("\n", fp);
}		

void printInterCodes(InterCode irList) {
	FILE *fp;
	if (strcmp(filename, "stdout") == 0) fp = stdout;
	else fp = fopen(filename, "w+");
	if (fp == NULL) {
		printf("printInterCode open file error!\n");
		return;
	}

	while (irList != NULL) {
		printInterCode(irList, fp);
		irList = irList->next;
	}

	fclose(fp);
}
