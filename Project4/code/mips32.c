#include "mips32.h"

RegDescripter regs[REG_NUM];
VarDescripter *varlist = NULL;

int args = 0;

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

int getReg(Operand op, FILE* fp) {
	char name[4];
	memset(name, 0,sizeof(name));

	VarDescripter* var = varlist;
	while (var != NULL) {
		if (op->kind == var->op->kind) {
			if (op->kind == TEMP && var->op->u.var_no == op->u.var_no) {
				sprintf(name, "t%d", op->u.var_no);
				break;
			}
		
			if (op->kind == VARIABLE && var->op->u.var_no == op->u.var_no) {
				sprintf(name, "v%d", op->u.var_no);
				break;
			}
		}
		
		var = var->next;
	}

	if (var == NULL) {
		var = (VarDescripter*)malloc(sizeof(VarDescripter));
		memset(var, 0, sizeof(VarDescripter));
		if (var == NULL) {
			printf("getReg error!\n");
			exit(1);
		}
		var->op = op;
		var->reg_no = -1;
		if (op->kind == TEMP || op->kind == VARIABLE) {
			var->next = varlist;
			varlist = var;
		}
	}

	if (var->reg_no == -1) {
		int reg_no = -1;
		for (int i = 0; i < 20; i++)
			if (regs[i].var != NULL) regs[i].old++;

		for (int i = 0; i < 20; i++)
			if (regs[i].var == NULL) {
				reg_no = i; break;
			}

		if (reg_no == -1) {
			int max = -1;
			for (int i = 0; i < 20; i++)
				if (regs[i].old > max) {
					max = regs[i].old;
					reg_no = i;
				}
		}

		var->reg_no = reg_no;
		regs[reg_no].var = var;
		if (var->op->kind == CONSTANT) {
			char str[32];
			memset(str, 0, sizeof(str));
			sprintf(str, "  li %s, %d\n", regs[var->reg_no].name, var->op->u.value);
			fputs(str, fp);
		}
	}
	regs[var->reg_no].old = 0;
	return var->reg_no;
}

StkDescripter stack;

void storeVarlist(FILE* fp) {
	VarDescripter* var = varlist;
	char str[32];
	while (var != NULL) {
		if (var->reg_no >= 0) {
//			stack.stack[stack.length] = var;
			fputs("  addi $sp, $sp, -4\n", fp);
			memset(str, 0, sizeof(str));
			sprintf(str, "  sw %s, 0($sp)\n", regs[var->reg_no].name);
			fputs(str, fp);
			stack.stack[stack.length].var = var;
			stack.stack[stack.length++].old = regs[var->reg_no].old;
//			stack.old[stack.length++] = regs[var->reg_no].old;
			regs[var->reg_no].old = 0;
			regs[var->reg_no].var = NULL;
		}
		var = var->next;
	}
	varlist = NULL;
}

void loadVarlist(FILE* fp) {
	VarDescripter* var = varlist;
	while (var != NULL) {
		varlist = var->next;
		free(var);
		var = varlist;
	}

	char str[32];
	for (int i = stack.length - 1; i >= 0; i--) {
		var = stack.stack[i].var;
		if (var == NULL) continue;

		var->next = varlist;
		varlist = var;

		memset(str,0, sizeof(str));
		sprintf(str, "  lw %s, 0($sp)\n", regs[var->reg_no].name);
		fputs(str, fp);
		fputs("  addi $sp, $sp, 4\n", fp);
		regs[var->reg_no].var = var;
		regs[var->reg_no].old = stack.stack[i].old;
	}

	stack.length = 0;
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
			sprintf(str, "  j label%d\n", ir->code.u.singleOP.op->u.var_no);
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
			if (args < 4) args++;
			getReg(ir->code.u.singleOP.op, fp);	
			break;
		case IR_CALL:
		{
			char str[32];
			memset(str, 0, sizeof(str));
			int off = -4 * args - 4;
			sprintf(str, "  addi $sp, $sp, %d\n", off);
			fputs(str, fp);
			fputs("  sw $ra, 0($sp)\n", fp);
			
			switch (args) {
				case 1:
				{
					fputs("  sw $a0, 4($sp)\n", fp);
					InterCode prev = ir->prev;
					int reg_no = getReg(prev->code.u.singleOP.op, fp);
					memset(str, 0, sizeof(str));
					sprintf(str, "  move $a0 %s\n", regs[reg_no].name);
					fputs(str, fp);

					storeVarlist(fp);
					memset(str, 0, sizeof(str));
					sprintf(str, "  jal %s\n", ir->code.u.doubleOP.right->u.name);
					fputs(str, fp);
					loadVarlist(fp);

					fputs("  lw $a0, 4($sp)\n", fp);
					fputs("  lw $ra, 0($sp)\n", fp);
					fputs("  addi $sp, $sp, 8\n", fp);
					break;
				}

				case 2:
				{
					fputs("  sw $a0, 4($sp)\n", fp);
					fputs("  sw $a1, 8($sp)\n", fp);

					char str[32];
					memset(str, 0, sizeof(str));

					InterCode prev1 = ir->prev;
					int reg_no1 = getReg(prev1->code.u.singleOP.op, fp);
					sprintf(str, "  move $a0 %s\n", regs[reg_no1].name);
					fputs(str, fp);

					InterCode prev2 = prev1->prev;
					int reg_no2 = getReg(prev2->code.u.singleOP.op, fp);
					memset(str, 0, sizeof(str));
					sprintf(str, "  move $a1 %s\n", regs[reg_no2].name);
					fputs(str, fp);

					storeVarlist(fp);
					memset(str, 0, sizeof(str));
					sprintf(str, "  jal %s\n", ir->code.u.doubleOP.right->u.name);
					fputs(str, fp);
					loadVarlist(fp);
					fputs("  lw $a1, 8($sp)\n", fp);
					fputs("  lw $a0, 4($sp)\n", fp);
					fputs("  lw $ra, 0($sp)\n", fp);
					fputs("  addi $sp, $sp, 12\n", fp);
					break;
				}
				case 3:
				{
					fputs("  sw $a0, 4($sp)\n", fp);
					fputs("  sw $a1, 8($sp)\n", fp);
					fputs("  sw $a2, 12($sp)\n", fp);

					char str[32];
					memset(str, 0, sizeof(str));

					InterCode prev1 = ir->prev;
					int reg_no1 = getReg(prev1->code.u.singleOP.op, fp);
					sprintf(str, "  move $a0 %s\n", regs[reg_no1].name);
					fputs(str, fp);

					InterCode prev2 = prev1->prev;
					int reg_no2 = getReg(prev2->code.u.singleOP.op, fp);
					memset(str, 0, sizeof(str));
					sprintf(str, "  move $a1 %s\n", regs[reg_no2].name);
					fputs(str, fp);

					InterCode prev3 = prev2->prev;
					int reg_no3 = getReg(prev3->code.u.singleOP.op, fp);
					memset(str, 0, sizeof(str));
					sprintf(str, "  move $a2 %s\n", regs[reg_no3].name);
					fputs(str, fp);

					storeVarlist(fp);
					memset(str, 0, sizeof(str));
					sprintf(str, "  jal %s\n", ir->code.u.doubleOP.right->u.name);
					fputs(str, fp);
					loadVarlist(fp);

					fputs("  lw $a2, 12($sp)\n", fp);
					fputs("  lw $a1, 8($sp)\n", fp);
					fputs("  lw $a0, 4($sp)\n", fp);
					fputs("  lw $ra, 0($sp)\n", fp);
					fputs("  addi $sp, $sp, 16\n", fp);
					break;
				}
				case 4:
				{
					fputs("  sw $a0, 4($sp)\n", fp);
					fputs("  sw $a1, 8($sp)\n", fp);
					fputs("  sw $a2, 12($sp)\n", fp);
					fputs("  sw $a3, 16($sp)\n", fp);

					char str[32];
					memset(str, 0, sizeof(str));

					InterCode prev1 = ir->prev;
					int reg_no1 = getReg(prev1->code.u.singleOP.op, fp);
					sprintf(str, "  move $a0 %s\n", regs[reg_no1].name);
					fputs(str, fp);

					InterCode prev2 = prev1->prev;
					int reg_no2 = getReg(prev2->code.u.singleOP.op, fp);
					memset(str, 0, sizeof(str));
					sprintf(str, "  move $a1 %s\n", regs[reg_no2].name);
					fputs(str, fp);

					InterCode prev3 = prev2->prev;
					int reg_no3 = getReg(prev3->code.u.singleOP.op, fp);
					memset(str, 0, sizeof(str));
					sprintf(str, "  move $a2 %s\n", regs[reg_no3].name);
					fputs(str, fp);

					InterCode prev4 = prev3->prev;
					int reg_no4 = getReg(prev3->code.u.singleOP.op, fp);
					memset(str, 0, sizeof(str));
					sprintf(str, "  move $a3 %s\n", regs[reg_no4].name);
					fputs(str, fp);

					storeVarlist(fp);
					memset(str, 0, sizeof(str));
					sprintf(str, "  jal %s\n", ir->code.u.doubleOP.right->u.name);
					fputs(str, fp);
					loadVarlist(fp);

					fputs("  lw $a3, 16($sp)\n", fp);
					fputs("  lw $a2, 12($sp)\n", fp);
					fputs("  lw $a1, 8($sp)\n", fp);
					fputs("  lw $a0, 4($sp)\n", fp);
					fputs("  lw $ra, 0($sp)\n", fp);
					fputs("  addi $sp, $sp, 20\n", fp);
					break;
				}
			}

			args = 0;
			int reg_no = getReg(ir->code.u.doubleOP.left, fp);
			memset(str, 0, sizeof(str));
			sprintf(str, "  move %s, $v0\n", regs[reg_no].name);
			fputs(str, fp);
			break;
		}	
		case IR_FUNCTION:
		{
			char str[32];
			memset(str, 0, sizeof(str));
			sprintf(str, "\n%s:\n", ir->code.u.singleOP.op->u.name);
			fputs(str, fp);

			VarDescripter *var = varlist;
			while (var != NULL) {
				varlist = var->next;
				free(var);
				var = varlist;
			}

			for (int i = 0; i < 20; i++) {
				regs[i].old = 0;
				regs[i].var = NULL;
			}

			int i = 0, reg_no;
			InterCode ir_t = ir->next;
			while (ir_t != NULL && ir_t->code.kind == IR_PARAM) {
				reg_no = getReg(ir_t->code.u.singleOP.op, fp);
				memset(str, 0, sizeof(str));
				sprintf(str, "  move %s, $a%d\n", regs[reg_no].name, i++);
				fputs(str, fp);
				ir_t = ir_t->next;
			}
			break;
		}
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
			sprintf(str, "  move %s, $v0\n", regs[reg_no].name);
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

	while (irList != NULL) {
		genMIPS(irList, fp);
		irList = irList->next;
	}
	
	fclose(fp);
}
