#include "mips32.h"

RegDescripter regs[REG_NUM];

void initMIPS() {
	strcpy(regs[0].name, "$t0");
	strcpy(regs[1].name, "$t1");
	strcpy(regs[2].name, "$t2");
	strcpy(regs[3].name, "$t3");
	strcpy(regs[4].name, "$t4");
	strcpy(regs[5].name, "$t5");
	strcpy(regs[6].name, "$t6");
	strcpy(regs[7].name, "$t7");
	strcpy(regs[8].name, "$t8");
	strcpy(regs[9].name, "$t9");
	strcpy(regs[10].name, "$s0");
	strcpy(regs[11].name, "$s1");
	strcpy(regs[12].name, "$s2");
	strcpy(regs[13].name, "$s3");
	strcpy(regs[14].name, "$s4");
	strcpy(regs[15].name, "$s5");
	strcpy(regs[16].name, "$s6");
	strcpy(regs[17].name, "$s7");

	strcpy(regs[18].name, "$v0");
	strcpy(regs[19].name, "$v1");
	strcpy(regs[20].name, "$a0");
	strcpy(regs[21].name, "$a1");
	strcpy(regs[22].name, "$a2");
	strcpy(regs[23].name, "$a3");

	strcpy(regs[24].name, "$fp");
	strcpy(regs[25].name, "$at");
	strcpy(regs[26].name, "$k0");
	strcpy(regs[27].name, "$k1");
	strcpy(regs[28].name, "$gp");
	strcpy(regs[29].name, "$sp");
	strcpy(regs[30].name, "$ra");
	strcpy(regs[31].name, "$zero");
}

void genMIPS(InterCode ir, FILE* fp) {
	if (ir == NULL) return;

	switch (ir->code.kind) {
		case IR_LABEL:
		{
			char str[32];
			memset(str, 0, sizeof(str));
			sprintf(str, "label%d:\n", ir->code.u.singleOP.op->u.var_no);
			fputs(str, fp);
			break;
		}
		case IR_ASSIGN:
		{
			Operand left = ir->code.u.doubleOP.left,
					right = ir->code.u.doubleOP.right;
			if (right->kind == CONSTANT) {
				int reg_no1 = getReg(left, fp);
				char str[32];
				memset(str, 0, sizeof(str));
				sprintf(str, "  li %s, %d\n", regs[reg_no1].name, right->u.value);
				fputs(str, fp);
			}
			else if (right->kind == TEMP || right->kind == VARIABLE) {
				int reg_no2 = getReg(right, fp),
					reg_no1 = getReg(left, fp);
				char str[32];
				memset(str, 0, sizeof(str));
				sprintf(str, "  move %s, %s\n", regs[reg_no1].name, regs[reg_no2].name);
				fputs(str, fp);
			}
			break;
		}
		case IR_ADD:
		case IR_SUB:
		{
			Operand result = ir->code.u.tripleOP.result,
					op1 = ir->code.u.tripleOP.op1,
					op2 = ir->code.u.tripleOP.op2;

			if (op2->kind == CONSTANT) {
				int reg_no2 = getReg(op1, fp),
					reg_no1 = getReg(result, fp);
				char str[32];
				memset(str, 0, sizeof(str));

				if (ir->code.kind == IR_ADD)
					sprintf(str, "  addi %s, %s, %d\n", regs[reg_no1].name, regs[reg_no2].name, op2->u.value);
				else if (ir->code.kind == IR_SUB)
					sprintf(str, "  addi %s, %s, -%d\n", regs[reg_no1].name, regs[reg_no2].name, op2->u.value);
				fputs(str, fp);
			}
			else {
				int reg_no2 = getReg(op1, fp),
					reg_no3 = getReg(op2, fp),
					reg_no1 = getReg(result, fp);
				char str[32];
				if (ir->code.kind == IR_ADD)
					sprintf(str, "  add %s, %s, %s\n", regs[reg_no1].name, regs[reg_no2].name, regs[reg_no3].name);
				else if (ir->code.kind == IR_SUB)
					sprintf(str, "  sub %s, %s, %s\n", regs[reg_no1].name, regs[reg_no2].name, regs[reg_no3].name);
				fputs(str, fp);
			}
			break;
		}
		case IR_MUL:
		case IR_DIV:
		{
			Operand result = ir->code.u.tripleOP.result,
					op1 = ir->code.u.tripleOP.op1,
					op2 = ir->code.u.tripleOP.op2;
			int reg_no2 = getReg(op1, fp),
				reg_no3 = getReg(op2, fp),
				reg_no1 = getReg(result, fp);
			char str[32];
			memset(str, 0, sizeof(str));
			if (ir->code.kind == IR_MUL) {
				sprintf(str, "  mul %s, %s, %s\n", regs[reg_no1].name, regs[reg_no2].name, regs[reg_no3].name);
				fputs(str, fp);
			}
			else if (ir->code.kind == IR_DIV) {
				sprintf(str, "  div %s, %s\n", regs[reg_no2].name, regs[reg_no3].name);
				fputs(str, fp);
				memset(str, 0, sizeof(str));
				sprintf(str, "  mflo %s\n", regs[reg_no1].name);
				fputs(str, fp);
			}
			break;
		}
		case IR_GOTO:
		{
			Operand op = ir->code.u.singleOP.op;
			char str[32];
			memset(str, 0, sizeof(str));
			sprintf(str, "  j label %d\n", ir->code.u.singleOP.op->u.var_no);
			fputs(str, fp);
			break;
		}
		case IR_IF:
		{
			int reg_no1 = getReg(ir->code.u.ifOP.op1, fp),
				reg_no2 = getReg(ir->code.u.ifOP.op2, fp);
			char* relop = ir->code.u.ifOP.relop;
			if (strcmp(relop, "==") == 0)
				relop = "beq";
			else if (strcmp(relop, "!=") == 0)
				relop = "bne";
			else if (strcmp(relop, ">") == 0)
				relop = "bgt";
			else if (strcmp(relop, "<") == 0)
				relop = "blt";
			else if (strcmp(relop, ">=") == 0)
				relop = "bge";
			else if (strcmp(relop, "<=") == 0)
				relop = "ble";

			char str[32];
			memset(str, 0, sizeof(str));
			sprintf(str, "  %s %s, %s, label%d\n", relop, regs[reg_no1].name, regs[reg_no2].name, ir->code.u.ifOP.label->u.var_no);
			fputs(str, fp);
			break;
		}
		case IR_RETURN:
		{
			Operand op = ir->code.u.singleOP.op;
			if (op->kind == CONSTANT && op->u.value == 0)
				fputs("  move $v0, $0\n", fp);
			else {
				int reg_no = getReg(op, fp);
				char str[32];
				memset(str, 0, sizeof(str));
				sprintf(str, "  move $v0, %s\n", regs[reg_no].name);
				fputs(str, fp);
			}
			fputs("  jr $ra\n", fp);
			break;
		}
		case IR_ARG:
			/* TODO */
			break;
			
		case IR_CALL:
			/* TODO */
			break;
			
		case IR_FUNCTION:
			/* TODO */
			break;

		case IR_READ:
		{
			fputs("  addi $sp, $sp, -4\n", fp);
			fputs("  sw $ra, 0($sp)\n", fp);
			fputs("  jal read\n", fp);
			fputs("  lw $ra, 0($sp)\n", fp);
			fputs("  addi $sp, $sp, 4\n", fp);
			int reg_no = getReg(ir->code.u.singleOP.op, fp);
			char str[32];
			memset(str, 0, sizeof(str));
			sprintf(str, "  move $%s, $v0\n", regs[reg_no].name);
			fputs(str, fp);
			break;
		}
		case IR_WRITE:
		{
			int reg_no = getReg(ir->code.u.singleOP.op, fp);
			char str[32];
			memset(str, 0, sizeof(str));
			sprintf(str, "  move $a0, %s\n", regs[reg_no].name);
			fputs(str, fp);
			fputs("  addi $sp, $sp, -4\n", fp);
			fputs("  sw $ra, 0($sp)\n", fp);
			fputs("  jal write\n", fp);
			fputs("  lw $ra, 0($sp)\n", fp);
			fputs("  addi $sp, $sp, 4\n", fp);
			break;
		}
	}
}

int getReg(Operand op, FILE* fp) {
	return 0;
}

void printMIPS(InterCode irList) {
	FILE* fp;
	if (strcmp(filename, "stdout") == 0) fp = stdout;
	else fp = fopen(filename, "w+");
	if (fp == NULL) {
			printf("printMIPS open file error!\n");
			return;
	}

	initMIPS();
	fputs(".data\n", fp);
	fputs("_prompt: .asciiz \"Enter an interger:\"\n", fp);
	fputs("_ret: .asciiz \"\\n\"\n", fp);
	fputs(".globl main\n", fp);
	fputs(".text\n\n", fp);

	fputs("read:\n", fp);
	fputs("  li $v0, 4\n", fp);
	fputs("  la $a0, _prompt\n", fp);
	fputs("  syscall\n", fp);
	fputs("  li $v0, 5\n", fp);
	fputs("  syscall\n", fp);
	fputs("  jr $ra\n\n", fp);

	fputs("write:\n", fp);
	fputs("  li $v0, 1\n", fp);
	fputs("  syscall\n", fp);
	fputs("  li $v0, 4\n", fp);
	fputs("  la $a0, _ret\n", fp);
	fputs("  syscall\n", fp);
	fputs("  move $v0, $0\n", fp);
	fputs("  jr $ra\n\n", fp);

	fputs("main:\n", fp);

	while (irList != NULL) {
		genMIPS(irList, fp);
		irList = irList->next;
	}
	
	fclose(fp);
}
