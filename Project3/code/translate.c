#include "ir.h"
#include "semantic.h"

extern SymbolList variable_table[HASH_TABLE_SIZE];
extern SymbolList function_table[HASH_TABLE_SIZE];

int temp_no = 1, var_no = 1, label_no = 1;

InterCode translate_Program(Node* root) {
	for (int i = 0; i < HASH_TABLE_SIZE; i++) {
		SymbolList p = variable_table[i];
		while (p != NULL) {
			p->dead = false; p = p->next;
		}
		p = function_table[i];
		while (p != NULL) {
			p->dead = false; p = p->next;
		}
	}
	
	// Program -> ExtDefList
	return translate_ExtDefList(root->child[0]);
}

InterCode translate_ExtDefList(Node* root) {
	if (root == NULL || root->childsum == 0) return NULL;
	// ExtDefList -> ExtDef ExtDefList
	InterCode code1 = translate_ExtDef(root->child[0]),
			  code2 = translate_ExtDefList(root->child[1]);
	return concat_code(code1, code2);
}

InterCode translate_ExtDef(Node* root) {
	// ExtDef -> Specifier ExtDecList SEMI
	if (strcmp(root->child[1]->name, "ExtDecList") == 0) {
		/* TODO */
	}

	else if (strcmp(root->child[1]->name, "FunDec") == 0) {
		// ExtDef -> Specifier FunDec CompSt
		if (strcmp(root->child[2]->name, "CompSt") == 0) {
			InterCode code1 = translate_FunDec(root->child[1]),
					  code2 = translate_CompSt(root->child[2]);
			return concat_code(code1, code2);
		}

		// ExtDef -> Specifier FunDec SEMI
		else {
			/* TODO */
		}
	}

	return NULL;
}

InterCode translate_VarDec(Node* root, Operand* op) {
	Node* VarDec = root;
	int i = 0;
	while (strcmp(VarDec->child[0]->name, "ID") != 0) {
		VarDec = VarDec->child[0]; i++;
	}

	// VarDec -> ID
	if (i == 0) {
		/* TODO */
	}

	else {
		switch(i) {
		// VarDec LB INT RB
			case 1:
			{
				/* TODO : 1 dimension array */
				break;
			}

			case 2:
			{
				/* TODO : 2 dimension array */
				break;
			}
		
			default: printf("no more operation for array dimension above 2\n"); break;
		}
	}

	return NULL;
}

InterCode translate_FunDec(Node* root) {
	/* TODO : function */
	// FunDec -> ID LP RP
	Operand op = getOP(searchSymbol(root->child[0]->text, function_table));
	InterCode ir = createInterCode(IR_FUNCTION, op);

	// FunDec -> ID LP VarList RP
	if (strcmp(root->child[2]->name, "VarList") == 0) {
		ir = concat_code(ir, translate_VarList(root->child[2]));	
	}

	return ir;
}

InterCode translate_VarList(Node* root) {
	/* TODO */
	// VarList -> ParamDec COMMA VarList

	// VarList -> ParamDec
	if 	(root->childsum < 3)
		return translate_ParamDec(root->child[0]);

	return NULL;
}

InterCode translate_ParamDec(Node* root) {
	// ParamDec -> Specifier VarDec
	Operand op = getOP(searchSymbol(root->child[1]->text, variable_table));
	return createInterCode(IR_PARAM, op); 
}

InterCode translate_CompSt(Node* root) {
	// CompSt -> LC DefList StmtList RC
	InterCode code1 = translate_DefList(root->child[1]),
			  code2 = translate_StmtList(root->child[2]);
	return concat_code(code1, code2);
}

InterCode translate_StmtList(Node* root) {
	if (root == NULL || root->childsum == 0) return NULL;
	// StmtList -> Stmt StmtList
	InterCode code1 = translate_Stmt(root->child[0]),
			  code2 = translate_StmtList(root->child[1]);
	return concat_code(code1, code2);
}

InterCode translate_Stmt(Node* root) {
	/* TODO : p83 */
	// Stmt -> Exp SEMI
	if (strcmp(root->child[0]->name, "Exp") == 0) {
		Operand op = NULL;
		return translate_Exp(root->child[0], &op);
	}

	// Stmt -> CompSt
	else if (strcmp(root->child[0]->name, "CompSt") == 0)
		return translate_CompSt(root->child[0]);

	// Stmt -> RETURN Exp SEMI
	else if (strcmp(root->child[0]->name, "RETURN") == 0) {
		Operand op = NULL; // = createOperand(TEMP, temp_no++);
		InterCode code1 = translate_Exp(root->child[1], &op),
				  code2 = createInterCode(IR_RETURN, op);
		return concat_code(code1, code2);
	}

	else if (strcmp(root->child[0]->name, "IF") == 0) {
		Operand label1 = createOperand(LABEL, label_no++),
				label2 = createOperand(LABEL, label_no++);
		InterCode code1 = translate_Cond(root->child[2], label1, label2),
				  code2 = translate_Stmt(root->child[4]);
		// Stmt -> IF LP Exp RP Stmt
		if (root->childsum == 5) 
			return concat_codes(4, code1, createInterCode(IR_LABEL, label1), code2, createInterCode(IR_LABEL, label2));
		// Stmt -> IF LP Exp RP Stmt1 ELSE Stmt2
		else {
			InterCode code3 = translate_Stmt(root->child[6]);
			if (strcmp(root->child[6]->child[0]->name, "RETURN") == 0)
				return concat_codes(5, code1, createInterCode(IR_LABEL, label1), code2, createInterCode(IR_LABEL, label2), code3);
			else {
				Operand label3 = createOperand(LABEL, label_no++);
				return concat_codes(7, code1, createInterCode(IR_LABEL, label1), code2, createInterCode(IR_GOTO, label3), createInterCode(IR_LABEL, label2), code3, createInterCode(IR_LABEL, label3));
			}
		}
	}

	else if (strcmp(root->child[0]->name, "WHIEL") == 0) {
		Operand label1 = createOperand(LABEL, label_no++),
				label2 = createOperand(LABEL, label_no++),
				label3 = createOperand(LABEL, label_no++);
		InterCode code1 = translate_Cond(root->child[2], label2, label3),
				  code2 = translate_Stmt(root->child[4]);
		return concat_codes(6, createInterCode(IR_LABEL, label1), code1, createInterCode(IR_LABEL, label2), code2, createInterCode(IR_GOTO, label1), createInterCode(IR_LABEL, label3));
	}
	
	return NULL;
}

InterCode translate_Cond(Node* root, Operand label_true, Operand label_false) {
	// Exp -> Exp RELOP Exp
	if (strcmp(root->child[1]->name, "RELOP") == 0) {
		Operand t1 = NULL, t2 = NULL;
		InterCode code1 = translate_Exp(root->child[0], &t1),
				  code2 = translate_Exp(root->child[2], &t2),
				  code3 = createInterCode(IR_IF, t1, t2, label_true, root->child[1]->text),
				  code4 = createInterCode(IR_GOTO, label_false);
		return concat_codes(4, code1, code2, code3, code4);
	}

	// Exp -> NOT Exp
	else if (strcmp(root->child[0]->name, "NOT") == 0)
		return translate_Cond(root, label_false, label_true);

	// Exp -> Exp1 AND Exp2
	else if (strcmp(root->child[1]->name, "AND") == 0) {
		Operand label = createOperand(LABEL, label_no++);
		InterCode code1 = translate_Cond(root->child[0], label, label_false),
				  code2 = translate_Cond(root->child[2], label_true, label_false);
		return concat_code(code1, code2);
	}

	// Exp -> Exp1 OR Exp2
	else if (strcmp(root->child[1]->name, "OR") == 0) {
		Operand label = createOperand(LABEL, label_no++);
		InterCode code1 = translate_Cond(root->child[0], label_true, label),
				  code2 = translate_Cond(root->child[2], label_true, label_false);
		return concat_code(code1, code2);
	}

	// (other case)
	Operand t = NULL;
	InterCode code1 = translate_Exp(root, &t),
			  code2 = createInterCode(IR_IF, t, createOperand(CONSTANT, 0), label_true, "!=");
	return concat_codes(3, code1, code2, createInterCode(IR_GOTO, label_false));
}

InterCode translate_DefList(Node* root) {
	// DefList -> Def DefList
	/*TODO*/

	return NULL;
}

InterCode translate_Def(Node* root) {
	// Def -> Specifier DecList SEMI
	return translate_DecList(root->child[1]);
}

InterCode translate_DecList(Node* root) {
	// DecList -> Dec
	InterCode code1 = translate_Dec(root->child[0]);
	
	// DecList -> Dec COMMA DecList
	if (root->childsum == 3) {
		InterCode code2 = translate_DecList(root->child[2]);
		code1 = concat_code(code1, code2);
	}

	return code1;
}

InterCode translate_Dec(Node* root) {
	Operand op1, op2;


	// Dec -> VarDec
	InterCode code1 = translate_VarDec(root->child[0], &op1);

	if (root->childsum == 3) {
		// Dec -> VarDec ASSIGNOP Exp
	}

	return code1;
}

InterCode translate_Exp(Node* root, Operand* op) {
	/* TODO : p82 */
	
	// Exp -> INT
	// Exp -> FLOAT
	if (strcmp(root->child[0]->name, "INT") == 0)
		*op = createOperand(CONSTANT, atoi(root->child[0]->text));
	else if (strcmp(root->child[0]->name, "FLOAT") == 0)
		*op = createOperand(FLOAT, atoi(root->child[0]->text));

	else if (strcmp(root->child[0]->name, "ID") == 0) {
		// Exp -> ID
		if (root->childsum == 1) {

			*op = getOP(searchSymbol(root->child[0]->text, variable_table));
			return NULL;
		}

			/* TODO:  search symbol */
		// Exp -> ID LP RP
		// Exp -> ID LP Args RP
		else /* TODO: function */ {
			if (strcmp(root->child[0]->text, "read") == 0) {
				*op = createOperand(TEMP, temp_no++);
				return createInterCode(IR_READ, *op);
			}
			else if (strcmp(root->child[0]->text, "write") == 0) {
				InterCode code = translate_Exp(root->child[2]->child[0], op);
				return concat_code(code, createInterCode(IR_WRITE, *op));
			}

			SymbolList func = searchSymbol(root->child[0]->text, function_table);
			Operand funcOP = getOP(func);
			int n = func->field->type.function->paramNum;
			Operand* arg_list = (Operand*)malloc(sizeof(Operand) * n);
			InterCode code1 = translate_Args(root->child[2], arg_list, n - 1),
					  code2 = NULL;
			for (int i = 0; i < n; i++)
				code2 = concat_code(code2, createInterCode(IR_ARG, arg_list[i]));
//			InterCode code1 = translate_Args(root->child[2]), code2 = NULL;
//			FieldList params = func->field->type.function->params;
//			for (int i = 0, n = func->field->type.function->paramNum; i < n; i++, params = params->tail)
//				code2 = concat_code(code2, createInterCode(IR_ARG, params->op));
			*op = createOperand(TEMP, temp_no++);
			return concat_codes(3, code1, code2, createInterCode(IR_CALL, *op, funcOP)); 
		}
	}

	// Exp -> Exp1 ASSIGNOP Exp2 (Exp1 -> ID)
	else if (strcmp(root->child[1]->name, "ASSIGNOP") == 0) {
		Operand v = getOP(searchSymbol(root->child[0]->child[0]->text, variable_table)),
				t = NULL; //createOperand(TEMP, temp_no++);
		InterCode code1 = translate_Exp(root->child[2], &t),
				  code2 = createInterCode(IR_ASSIGN, v, t);
		if (*op != NULL)
			code2 = concat_code(code2, createInterCode(IR_ASSIGN, *op, v));
		return concat_code(code1, code2);
	}
	
	// Exp -> Exp1 RELOP Exp2
	// Exp -> Exp AND Exp
	// Exp -> Exp OR Exp
	// Exp -> NOT Exp
	else if ((strcmp(root->child[1]->name, "RELOP") == 0) || (strcmp(root->child[1]->name, "AND") == 0) || (strcmp(root->child[1]->name, "OR") == 0) || (strcmp(root->child[0]->name, "NOT") == 0)) {
		Operand label1 = createOperand(LABEL, label_no++),
				label2 = createOperand(LABEL, label_no++);
		InterCode code0 = createInterCode(IR_ASSIGN, *op, createOperand(CONSTANT, 0)),
				  code1 = translate_Cond(root, label1, label2),
				  code2 = concat_code(createInterCode(IR_LABEL, label1), createInterCode(IR_ASSIGN, *op, createOperand(CONSTANT, 1)));
		return concat_codes(4, code0, code1, code2, createInterCode(IR_LABEL, label2));
	}

	// Exp -> Exp PLUS Exp
	// Exp -> Exp MINUS Exp
	// Exp -> Exp STAR Exp
	// Exp -> Exp DIV Exp
	else if ((strcmp(root->child[1]->name, "PLUS") == 0) || (strcmp(root->child[1]->name, "MINUS") == 0) || (strcmp(root->child[1]->name, "STAR") == 0) || (strcmp(root->child[1]->name, "DIV") == 0)) {

		Operand t1 = NULL, t2 = NULL;
//		Operand t1 = createOperand(TEMP, temp_no++),
//				t2 = createOperand(TEMP, temp_no++);
		InterCode code1 = translate_Exp(root->child[0], &t1),
				  code2 = translate_Exp(root->child[2], &t2),
				  code3 = NULL;
		*op = createOperand(TEMP, temp_no++);
		if (strcmp(root->child[1]->name, "PLUS") == 0)
			code3 = createInterCode(IR_ADD, *op, t1, t2);
		else if (strcmp(root->child[1]->name, "MINUS") == 0)
			code3 = createInterCode(IR_SUB, *op, t1, t2);
		else if (strcmp(root->child[1]->name, "STAR") == 0)
			code3 = createInterCode(IR_MUL, *op, t1, t2);
		else if (strcmp(root->child[1]->name, "DIV") == 0)
			code3 = createInterCode(IR_DIV, *op, t1, t2);
		return concat_codes(3, code1, code2, code3);
	}

	// Exp -> MINUS Exp
	else if (strcmp(root->child[0]->name, "MINUS") == 0) {
		Operand t = NULL;
		InterCode code1 = translate_Exp(root->child[1], &t);
		if (*op == NULL) *op = createOperand(TEMP, temp_no++);
		InterCode code2 = createInterCode(IR_SUB, *op, createOperand(CONSTANT, 0), t);
		return concat_code(code1, code2);
	}

	// Exp -> LP Exp RP
	else if (strcmp(root->child[0]->name, "LP") == 0) 
		return translate_Exp(root->child[1], op);

	// Exp -> Exp LB Exp RB
	else if (strcmp(root->child[1]->name, "LB") == 0) {
		/* TODO */
	}

	// Exp -> Exp DOT ID
	else if (strcmp(root->child[1]->name, "DOT") == 0) {
		/* TODO */
	}

	return NULL;
}

InterCode translate_Args(Node* root, Operand* arg_list, int i) {
	// Args -> Exp
	//Operand t = createOperand(TEMP, temp_no++);
	Operand t = NULL;
	InterCode code1 = translate_Exp(root->child[0], &t);
	arg_list[i] = t;
	
	// Args -> Exp COMMA Args
	if (root->childsum != 1) {
		InterCode code2 = translate_Args(root->child[2], arg_list, i - 1);
		code1 = concat_code(code1, code2);
	}

	return code1;
}
