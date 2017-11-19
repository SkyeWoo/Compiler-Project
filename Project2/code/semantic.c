#include "semantic.h"

bool VarTypeEqual(VarType type1, VarType type2) {
	if ((type1 == NULL) && (type2 == NULL)) return true;
	if ((type1 == NULL) || (type2 == NULL)) return false;

	if (type1->kind != type2->kind) return false;

	switch(type1->kind) {
		case BASIC:
			if (type1->u.basic.type == type2->u.basic.type) return true;
			else return false;
			break;
		case ARRAY:
			return VarTypeEqual(type1->u.array.elem, type2->u.array.elem);
			break;
		case STRUCTURE:
		{
			FieldList field1 = type1->u.structure;
			FieldList field2 = type2->u.structure;
			if ((field1 != NULL) && (field2 != NULL)) {
				while ((field1 != NULL) && (field2 != NULL)) {
					if (VarTypeEqual(field1->type.variable, field2->type.variable) == false)
						return false;
					field1 = field1->tail;
					field2 = field2->tail;
				}
				if ((field1 == NULL) && (field2 == NULL)) return true;
			}
			return false;
			break;
		}
		default: return false; break;
	}
}

// REQUIRE 3
bool FuncTypeEqual(FuncType type1, FuncType type2) {
	if (type1->paramNum != type2->paramNum) return false;
	FieldList param1 = type1->params,
			  param2 = type2->params;
	for (int i = 0; i < type1->paramNum; i++) {
		if (VarTypeEqual(param1->type.variable, param2->type.variable) == false)
			return false;
		param1 = param1->tail;
		param2 = param2->tail;
	}
	return true;
}

// REQUIRE 1
void check_declared_undefined() {
	for (int i = 0; i < HASH_TABLE_SIZE; i++)
		if (function_table[i] != NULL) {
			if (function_table[i]->field->type.function->defined == false) {
				printf("Error type 18 at Line %d: Undefined function \"%s\".\n", function_table[i]->lineno, function_table[i]->field->name);
			}
		}
}

void handle_Program(Node* root) {
	if (root == NULL) return;

	// Program -> ExtDefList
	if (root->childsum != 0)
		handle_ExtDefList(root->child[0]);

	check_declared_undefined();
}

void handle_ExtDefList(Node* root) {
	Node* ExtDefList = root;

	// ExtDefList -> ExtDef ExtDefList
	while (ExtDefList->childsum != 0) {
		handle_ExtDef(ExtDefList->child[0]);			

		if (ExtDefList->child[1] == NULL) return;

		ExtDefList = ExtDefList->child[1];
	}
}

void handle_ExtDef(Node* root) {
	Node* ExtDef = root;

	// the first element of ExtDef is Specifier
	VarType basic = handle_Specifier(ExtDef->child[0]);

	// ExtDef -> Specifier ExtDecList SEMI
	if (strcmp(ExtDef->child[1]->name, "ExtDecList") == 0) {
		handle_ExtDecList(ExtDef->child[1], basic);
	}

	else if (strcmp(ExtDef->child[1]->name, "FunDec") == 0) {
		// ExtDef -> Specifier FunDec CompSt
		if (strcmp(ExtDef->child[2]->name, "CompSt") == 0) {
			// true means defined, false means declared
			handle_FunDec(ExtDef->child[1], ExtDef, basic, true);
			handle_CompSt(ExtDef->child[2], basic);
		}
// REQUIRE 1
		// ExtDef -> Specifier FunDec SEMI
		else handle_FunDec(ExtDef->child[1], ExtDef, basic, false);
	}

	// Specifier SEMI
	else {}
}

void handle_ExtDecList(Node* root, VarType basic) {
	Node* ExtDecList = root;
//	FieldList field;
	SymbolList symbol;

	// ExtDecList -> VarDec COMMA ExtDecList
	while (ExtDecList->childsum == 3) {
		symbol = handle_VarDec(ExtDecList->child[0], basic);
		if (searchSymbol(symbol->field->name, variable_table) != NULL)
			printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", ExtDecList->lineno, symbol->field->name);
		else insertSymbol(symbol, variable_table);
		ExtDecList = ExtDecList->child[2];
	}

	// ExtDecList -> VarDec
	symbol = handle_VarDec(ExtDecList->child[0], basic);
	if (searchSymbol(symbol->field->name, variable_table) != NULL)
		printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", ExtDecList->lineno, symbol->field->name);
	else insertSymbol(symbol, variable_table);
}

VarType handle_Specifier(Node* root) {
	VarType Specifier = (VarType)malloc(sizeof(VarType_));
	
	// Specifier -> TYPE
	if (strcmp(root->child[0]->name, "TYPE") == 0) {
		Specifier->kind = BASIC;
		if (strcmp(root->child[0]->text, "int") == 0)
			Specifier->u.basic.type = INT;
		else Specifier->u.basic.type = FLOAT;
		return Specifier;
	}
	
	// Specifier -> StructSpecifier
	else {
		Specifier->kind = STRUCTURE;
		Node* StructSpecifier = root->child[0];
	
		// StructSpecifier -> STRUCT Tag
		if (StructSpecifier->childsum == 2) {
			char* s = StructSpecifier->child[1]->child[0]->text; // ID
			SymbolList symbol = searchSymbol(s, variable_table);
			if (symbol == NULL) {
				printf("Error type 17 at Line %d: Undefined structure \"%s\".\n", root->lineno, s);
				Specifier->u.structure = NULL;
				return Specifier;
			}
			else if (symbol->field->type.variable != NULL)
				return symbol->field->type.variable;
			Specifier->u.structure = NULL;
			return Specifier;
		}

		// StructSpecifier -> STRUCT OptTag LC DefList RC
		else {
			Node* DefList = root->child[0]->child[3];
			Specifier->u.structure = NULL;

			// DefList -> Def DefList
			while (DefList != NULL) {
				Node* Def = DefList->child[0];
				VarType basic = handle_Specifier(Def->child[0]);

				Node* DecList = Def->child[1];
				// DecList -> Dec COMMA DecList
				while (DecList->childsum == 3) {
					SymbolList symbol = handle_VarDec(DecList->child[0]->child[0], Specifier);
					if (DecList->child[0]->childsum != 1)
						printf("Error type 15 at Line %d: Variable %s in struct is initialized.\n", Def->lineno, symbol->field->name);
					FieldList temp = Specifier->u.structure;
					while (temp != NULL) {
						if (strcmp(temp->name, symbol->field->name) == 0) {
							printf("Error type 15 at Line %d: Redefined field \"%s\".\n", Def->lineno, symbol->field->name);
							break;
						}
						temp = temp->tail;
					}

					if (temp == NULL) {
						if (searchSymbol(symbol->field->name, variable_table) != NULL)
							printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", Def->lineno, symbol->field->name);
						else {
							insertSymbol(symbol, variable_table);
							symbol->field->tail = Specifier->u.structure;
							Specifier->u.structure = symbol->field;
						}
					}
					DecList = DecList->child[2];
				}

				SymbolList symbol = handle_VarDec(DecList->child[0]->child[0], basic);
				if (DecList->child[0]->childsum != 1)
					printf("Error type 15 at Line %d: Variable \"%s\" in struct is initialized.\n", Def->lineno, symbol->field->name);
				FieldList temp = Specifier->u.structure;
				while (temp != NULL) {
					if (strcmp(temp->name, symbol->field->name) == 0) {
						printf("Error type 15 at Line %d: Redefined field \"%s\".\n", Def->lineno, symbol->field->name);
						break;
					}
					temp = temp->tail;
				}
				if (temp == NULL) {
					if (searchSymbol(symbol->field->name, variable_table) != NULL)
						printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", Def->lineno, symbol->field->name);
					else {
						insertSymbol(symbol, variable_table);
						symbol->field->tail = Specifier->u.structure;
						Specifier->u.structure = symbol->field;
					}
				}
				DefList = DefList->child[1];
			}

			// OptTag -> ID
			if (StructSpecifier->child[1] != NULL) {
				SymbolList symbol = (SymbolList)malloc(sizeof(SymbolList_));
				symbol->field = (FieldList)malloc(sizeof(FieldList_));
				symbol->lineno = StructSpecifier->lineno;
				symbol->field->type.variable = Specifier;
				symbol->field->name = StructSpecifier->child[1]->child[0]->text; // ID
				if (searchSymbol(symbol->field->name, variable_table) != NULL)
					printf("Error type 16 at Line %d: Duplicated name \"%s\".\n", root->lineno, symbol->field->name);
				else insertSymbol(symbol, variable_table);
			}
			/* TODO <anoymous name> struct */

			return Specifier;
		}
	}
}

SymbolList handle_VarDec(Node* root, VarType basic) {
	Node* VarDec = root;
	// in case VarDec is an array
	// i remember the array's dimemsion
	int i = 0;
	while (strcmp(VarDec->child[0]->name, "ID") != 0) {
		VarDec = VarDec->child[0]; i++;
	}

	SymbolList symbol = (SymbolList)malloc(sizeof(SymbolList_));
	symbol->field = (FieldList)malloc(sizeof(FieldList_));
	symbol->field->name = VarDec->child[0]->text;
	symbol->lineno = root->lineno;

	// VarDec -> ID
	if (i == 0) {
		symbol->field->type.variable = basic;
		return symbol;
	}

	VarType var1 = (VarType)malloc(sizeof(VarType_));
	var1->kind = ARRAY;
	var1->u.array.size = atoi(root->child[2]->text);
	var1->u.array.elem = basic;

	switch(i) {
		case 1: 
			symbol->field->type.variable = var1;
			return symbol;
		case 2: {
			VarType var2 = (VarType)malloc(sizeof(VarType_));
			var2->kind = ARRAY;
			var2->u.array.size = atoi(root->child[0]->child[2]->text);
			var2->u.array.elem = var1;
			symbol->field->type.variable = var2;
			return symbol;
		}
		default: printf("no more operation for array dimension above 2\n"); break;
	}
}

void handle_FunDec(Node* root, Node* ExtDef, VarType basic, bool defined) {
	SymbolList symbol = (SymbolList)malloc(sizeof(SymbolList_));
	symbol->field = (FieldList)malloc(sizeof(FieldList_));
	symbol->field->name = root->child[0]->text;
	symbol->lineno = root->lineno;
	FuncType type = (FuncType)malloc(sizeof(FuncType_));
	type->rtnType = basic;
	type->defined = defined;

	// FunDec -> ID LP RP
	type->params = NULL;
	type->paramNum = 0;

	// FunDec -> ID LP VarList RP
	if (strcmp(root->child[2]->name, "VarList") == 0) {
		Node* VarList = root->child[2];
		// VarList -> ParamDec COMMA VarList
		while (VarList->childsum == 3) {
			SymbolList ParamDec_VarDec = handle_ParamDec(VarList->child[0], ExtDef);
			type->paramNum++;
			ParamDec_VarDec->field->tail = type->params;
			type->params = ParamDec_VarDec->field;

			VarList = VarList->child[2];
		}

		// VarList -> ParamDec
		SymbolList ParamDec_VarDec = handle_ParamDec(VarList->child[0], ExtDef);	
		type->paramNum++;
		ParamDec_VarDec->field->tail = type->params;
		type->params = ParamDec_VarDec->field;
	}

	symbol->field->type.function = type;

	SymbolList p;
	if ((p = searchSymbol(symbol->field->name, function_table)) != NULL) {
// REQUIRE 1
		if (defined == true && p->field->type.function->defined == true) printf("Error type 4 at Line %d: Redefined function \"%s\".\n", ExtDef->lineno, p->field->name);
		else {
			if (defined == true) p->field->type.function->defined = true;
			bool flag = FuncTypeEqual(p->field->type.function, type);
			if (defined == false && flag == false)
				printf("Error type1 19 at Line %d: Inconsistent declaration of function \"%s\".\n", ExtDef->lineno, symbol->field->name);

		}
	}
	else insertSymbol(symbol, function_table);
}

SymbolList handle_ParamDec(Node* root, Node* ExtDef) {
	// ParamDec -> Specifier VarDec
	VarType Specifier = handle_Specifier(root->child[0]);
	SymbolList VarDec = handle_VarDec(root->child[1], Specifier);
	SymbolList p;
	if ((p = searchSymbol(VarDec->field->name, variable_table)) != NULL) {
// REQUIRE 1
		if (VarTypeEqual(p->field->type.variable, VarDec->field->type.variable) == false)
			printf("Error type2 19 at Line %d: Inconsistent declaration of function \"%s\".\n", root->lineno, VarDec->field->name);
	//	printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", ExtDef->lineno, VarDec->name);
	}
	else insertSymbol(VarDec, variable_table);

	return VarDec;
}

void handle_CompSt(Node* root, VarType rtnType) {
	Node* CompSt = root;

	// CompSt -> LC DefList StmtList RC
	handle_DefList(CompSt->child[1]);
	handle_StmtList(CompSt->child[2], rtnType);
}

void handle_StmtList(Node* root, VarType rtnType) {
	Node* StmtList = root;

	// StmtList -> Stmt StmtList
	while (StmtList != NULL) {
		handle_Stmt(StmtList->child[0], rtnType);
		StmtList = StmtList->child[1];
	}
}

void handle_Stmt(Node* root, VarType rtnType) {
	Node* Stmt = root;

	// Stmt -> Exp SEMI
	if (strcmp(Stmt->child[0]->name, "Exp") == 0)
		handle_Exp(Stmt->child[0]);

	// Stmt -> CompSt
	else if (strcmp(Stmt->child[0]->name, "CompSt") == 0)
		handle_CompSt(Stmt->child[0], rtnType);
	
	// Stmt -> RETURN Exp SEMI
	else if (strcmp(Stmt->child[0]->name, "RETURN") == 0) {
		VarType tempRtnType = handle_Exp(Stmt->child[1]);
		if (VarTypeEqual(rtnType, tempRtnType) == false)
			printf("Error type 8 at Line %d: VarType mismatched for return.\n", Stmt->lineno);
	}

	// Stmt -> WHILE LP Exp RP Stmt
	else if (strcmp(Stmt->child[0]->name, "WHILE") == 0) {
		VarType type = handle_Exp(Stmt->child[2]);
		if (!((type->kind == BASIC) && (type->u.basic.type == INT)))
			printf("Error type 5 at Line %d: Only type INT could be used for judgement.\n", Stmt->lineno);
		handle_Stmt(Stmt->child[4], rtnType);
	}

	// Stmt -> IF LP Exp RP Stmt
	else if (Stmt->childsum == 5) {
		VarType type = handle_Exp(Stmt->child[2]);
		if (type != NULL)
			if (!((type->kind == BASIC) && (type->u.basic.type == INT)))
				printf("Error type 5 at Line %d: Only type INT could be used for judgement.\n", Stmt->lineno);

		handle_Stmt(Stmt->child[4], rtnType);
	}

	// Stmt -> IF LP Exp RP Stmt ELSE Stmt 
	else {
		VarType type = handle_Exp(Stmt->child[2]);
		if (!((type->kind == BASIC) && (type->u.basic.type == INT)))
			printf("Error type 5 at Line %d: Only type INT could be used for judgement.\n", Stmt->lineno);
		handle_Stmt(Stmt->child[4], rtnType);
		handle_Stmt(Stmt->child[6], rtnType);
	}
}

void handle_DefList(Node* root) {
	Node* DefList = root;
	
	// DefList -> Def DefList
	while (DefList != NULL) {
		handle_Def(DefList->child[0]);

		if (DefList->child[1] == NULL) return;

		DefList = DefList->child[1];
	}
}

void handle_Def(Node* root) {
	Node* Def = root;

	// Def -> Specifier DecList SEMI
	VarType basic = handle_Specifier(Def->child[0]);
	handle_DecList(Def->child[1], basic);
}

void handle_DecList(Node* root, VarType basic) {
	Node* DecList = root;

	// DecList -> Dec COMMA DecList
	while (DecList->childsum == 3) {
		handle_Dec(DecList->child[0], DecList, basic);
		DecList = DecList->child[2];
	}
	handle_Dec(DecList->child[0], DecList, basic);
}

void handle_Dec(Node* root, Node* DecList, VarType basic) {
	Node* Dec = root;

	// Dec -> VarDec
	SymbolList symbol = handle_VarDec(root->child[0], basic);
	if (searchSymbol(symbol->field->name, variable_table) != NULL)
		printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", DecList->lineno, symbol->field->name);
	else insertSymbol(symbol, variable_table);

	/* TODO */
}

VarType handle_Exp(Node* root) {
	if (root == NULL) return NULL;

	// handle orderd by paramsNum

	// Exp -> ID
	if ((strcmp(root->child[0]->name, "ID") == 0) && (root->childsum == 1)) {
		SymbolList symbol = searchSymbol(root->child[0]->text, variable_table);
		if (symbol != NULL) return symbol->field->type.variable;
		else {
			printf("Error type 1 at Line %d: Undefined variable \"%s\".\n", root->lineno, root->child[0]->text);
			return NULL;
		}
	}

	// Exp -> INT
	else if (strcmp(root->child[0]->name, "INT") == 0) {
		VarType type = (VarType)malloc(sizeof(VarType_));
		type->kind = BASIC;
		type->u.basic.type = INT;
		type->u.basic.name = root->child[0]->text;
		return type;
	}

	// Exp -> FLOAT
	else if (strcmp(root->child[0]->name, "FLOAT") == 0) {
		VarType type = (VarType)malloc(sizeof(VarType_));
		type->kind = BASIC;
		type->u.basic.type = FLOAT;
		type->u.basic.name = root->child[0]->text;
		return type;
	}

	// Exp -> Exp ASSIGNOP Exp
	else if (strcmp(root->child[1]->name, "ASSIGNOP") == 0) {
		if (root->child[0]->childsum == 1) {
			// subExp -> INT / FLOAT
			if (!(strcmp(root->child[0]->child[0]->name, "ID") == 0)) {
				printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n", root->lineno);
				return NULL;
			}
			// subExp -> ID
			else {}
		}
		else if (root->child[0]->childsum == 3) {
			if (!((strcmp(root->child[0]->child[0]->name, "Exp") == 0) && (strcmp(root->child[0]->child[1]->name, "DOT") == 0) && (strcmp(root->child[0]->child[2]->name, "ID") == 0))) {
				printf("Error type 6 at Line %d: The left-hand side of an assignemnt must be a variable.\n", root->lineno);
				return NULL;
			}
			// subExp -> Exp DOT ID
			else {}
		}
		else if (root->child[0]->childsum == 4) {
			if (!((strcmp(root->child[0]->child[0]->name, "Exp") == 0) && (strcmp(root->child[0]->child[1]->name, "LB") == 0) && (strcmp(root->child[0]->child[2]->name, "Exp") == 0) && (strcmp(root->child[0]->child[3]->name, "RB") == 0))) {
				printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n", root->lineno);
				return NULL;
			}
			// subExp -> Exp LB Exp RB
			else {}
		}

		// TODO: bug!!
		VarType left = handle_Exp(root->child[0]),
				right = handle_Exp(root->child[2]);
		if (VarTypeEqual(left, right) == false) {
			if ((left != NULL) && (right != NULL))
				printf("Error type 5 at Line %d: VarType mismatched for assignment.\n", root->lineno);
			return NULL;
		}
		else return left;
	}
	
	// Exp -> Exp AND Exp
	// Exp -> Exp OR Exp
	// Exp -> Exp RELOP Exp
	else if ((strcmp(root->child[1]->name, "AND") == 0) || (strcmp(root->child[1]->name, "OR") == 0) || (strcmp(root->child[1]->name, "RELOP") == 0)) {
		VarType left = handle_Exp(root->child[0]),
				right = handle_Exp(root->child[2]);
		if (VarTypeEqual(left, right) == false) {
			if ((left != NULL) && (right != NULL))
				printf("Error type 7 at Line %d: VarType mismatched for operands.\n", root->lineno);
			return NULL;
		}
		else {
			VarType type = (VarType)malloc(sizeof(VarType_));
			type->kind = BASIC;
			type->u.basic.type = INT;
			return type;
		}
	}

	// Exp -> Exp PLUS Exp
	// Exp -> Exp MINUS Exp
	// Exp -> Exp STAR Exp
	// Exp -> Exp DIV Exp
	else if ((strcmp(root->child[1]->name, "PLUS") == 0) || (strcmp(root->child[1]->name, "MINUS") == 0) || (strcmp(root->child[1]->name, "STAR") == 0) || (strcmp(root->child[1]->name, "DIV") == 0)) {
		VarType left = handle_Exp(root->child[0]),
				right = handle_Exp(root->child[2]);
		if (VarTypeEqual(left, right) == false) {
			if ((left != NULL) && (right != NULL))
				printf("Error type 7 at Line %d: VarType mismatched for operands.\n", root->lineno);
			return NULL;
		}
		else return left;
	}

	// Exp -> LP Exp RP
	// Exp -> MINUS Exp
	// Exp -> NOT Exp
	else if ((strcmp(root->child[0]->name, "LP") == 0) || (strcmp(root->child[0]->name, "MINUS") == 0) || (strcmp(root->child[0]->name, "NOT") == 0))
		return handle_Exp(root->child[1]);

	// Exp -> ID LP RP
	// Exp -> ID LP Args RP
	else if (strcmp(root->child[0]->name, "ID") == 0) {
		SymbolList symbol = searchSymbol(root->child[0]->text, function_table);
		if (symbol == NULL) {
			SymbolList s = searchSymbol(root->child[0]->text, variable_table);
			if (s != NULL)
				printf("Error type 11 at Line %d: \"%s\" is not a function.\n", root->lineno, root->child[0]->text);
			else printf("Error type 2 at Line %d: Undefined function \"%s\".\n", root->lineno, root->child[0]->text);
			return NULL;
		}

		FuncType funcType = symbol->field->type.function;
		FuncType type = (FuncType)malloc(sizeof(FuncType_));
		type->paramNum = 0;
		type->params = NULL;
// REQUIRE 1
		type->defined = funcType->defined;

		// Exp -> ID LP Args RP
		if (strcmp(root->child[2]->name, "Args") == 0)
			handle_Args(root->child[2], &type);

		if (FuncTypeEqual(type, funcType) == false) {
			printf("Error type 9 at Line %d: Params mismatched in function \"%s\".\n", root->lineno, root->child[0]->text);
			return NULL;
		}

		else return funcType->rtnType;
	}

	// Exp -> Exp LB Exp RB
	else if (strcmp(root->child[1]->name, "LB") == 0) {
		VarType Exp = handle_Exp(root->child[0]);
		if (Exp->kind != ARRAY) {
			Node* temp = root->child[0];
			char* s;

			switch(temp->childsum) {
				case 1: // Exp -> ID
					if (strcmp(temp->child[0]->name, "ID") == 0)
						s = temp->child[0]->text;
					break;
				case 3: // Exp -> Exp DOT ID
					if (strcmp(temp->child[2]->name, "ID") == 0)
						s = temp->child[0]->text;
					break;
				case 4:
					if (strcmp(temp->child[0]->name, "Exp") == 0)
						if (strcmp(temp->child[0]->child[0]->name, "ID") == 0)
							s = temp->child[0]->child[0]->text;
					break;
				default: printf("no more operation 1\n"); break;
			}

			if (searchSymbol(s, variable_table) != NULL)
				printf("Error type 10 at Line %d: \"%s\" is not an array.\n", root->lineno, s);
			return NULL;
		}

		VarType Exp2 = handle_Exp(root->child[2]);
		if (Exp2->kind != BASIC || (Exp2->kind == BASIC && Exp2->u.basic.type != INT)) {
			printf("Error type 12 at Line %d: \"%s\" is not an integer.\n", root->lineno, Exp2->u.basic.name);
			return NULL;
		}

		return Exp->u.array.elem;
	}

	// Exp -> Exp DOT ID
	else if (strcmp(root->child[1]->name, "DOT") == 0) {
		VarType Exp = handle_Exp(root->child[0]);
		if (Exp->kind != STRUCTURE) {
			Node* temp = root->child[0];
			char* s;
			switch(temp->childsum) {
				case 1: // subExp -> ID
					if (strcmp(temp->child[0]->name, "ID") == 0)
						s = temp->child[0]->text;
					break;
				case 3: // subExp -> Exp DOT ID
					if (strcmp(temp->child[2]->name, "ID") == 0)
						s = temp->child[0]->text;
					break;
				case 4: // subExp -> Exp LB Exp RB
					if (strcmp(temp->child[0]->name, "Exp") == 0)
						if (strcmp(temp->child[0]->child[0]->name, "ID") == 0)
							s = temp->child[0]->child[0]->text;
					break;
				default: printf("no more operation\n"); break;
			}
			
			if (searchSymbol(s, variable_table) != NULL)
				printf("Error type 13 at Line %d: Illegal use of \".\".\n", root->lineno);
			return NULL;
		}

		char* s = root->child[2]->text;
		FieldList temp = Exp->u.structure;
		while (temp != NULL) {
			if (strcmp(temp->name, s) == 0) return temp->type.variable;
			temp = temp->tail;
		}

		printf("Error type 14 at Line %d: Non-existent field \"%s\".\n", root->lineno, root->child[2]->text);
		return NULL;
	}

	// no error!
	else return NULL;
}

void handle_Args(Node* root, FuncType* type) {
	Node* Args = root;

	// Args -> Exp COMMA Args
	while (Args->childsum != 1) {
		VarType Exp = handle_Exp(Args->child[0]);
		FieldList field = (FieldList)malloc(sizeof(FieldList_));
		field->type.variable = Exp;
		field->tail = (*type)->params;
		(*type)->paramNum++;
		(*type)->params = field;

		Args = Args->child[2];
	}

	// Args -> Exp
	VarType Exp = handle_Exp(Args->child[0]);
	FieldList field = (FieldList)malloc(sizeof(FieldList_));
	field->type.variable = Exp;
	field->tail = (*type)->params;
	(*type)->paramNum++;
	(*type)->params = field;
}
