#include "semantic.h"

unsigned int hash_pjw(char* name) {
	unsigned int val = 0, i;
	for (; *name; ++name) {
		val = (val << 2) + *name;
		if (i = val & ~0x3fff) val = (val ^ (i >> 12)) & 0x3fff;
	}
	return val;
}

#define HASH_TABLE_SIZE 16384
FieldList variable_table[HASH_TABLE_SIZE];
FieldList function_table[HASH_TABLE_SIZE];

void initTable() {
	for (int i = 0; i < HASH_TABLE_SIZE; i++) {
		variable_table[HASH_TABLE_SIZE] = NULL;
		function_table[HASH_TABLE_SIZE] = NULL;
		//hashTable[i] = NULL;
	}
}


bool insertSymbol(FieldList f, FieldList* table) {
	if (f == NULL || f->name == NULL) return false;

	unsigned int key = hash_pjw(f->name);

	// no collision
	if (table[key] == NULL) {
		table[key] = f; return true;
	}

	// collision
	while (1) {
		key = (++key) % HASH_TABLE_SIZE;
		FieldList q = table[key];
		if (q == NULL) {
			table[key] = f; return true;
		}
	}

	return false;
}

FieldList searchSymbol(char* name, FieldList* table) {
	if (name == NULL) return NULL;

	unsigned int key = hash_pjw(name);

	FieldList p = table[key];
	while (p != NULL) {
		if (strcmp(name, p->name) == 0) return p;
		key = (++key) % HASH_TABLE_SIZE;
		p = table[key];
	}

	return NULL;
}

bool TypeEqual(Type type1, Type type2) {
	if ((type1 == NULL) && (type2 == NULL)) return true;
	if ((type1 == NULL) || (type2 == NULL)) return false;

	if (type1->kind != type2->kind) return false;

	switch(type1->kind) {
		case BASIC:
			if (type1->u.basic.type == type2->u.basic.type) return true;
			else return false;
			break;
		case ARRAY:
			return TypeEqual(type1->u.array.elem, type2->u.array.elem);
			break;
		case STRUCTURE:
			return false;
			/* TODO */
			break;
		case FUNCTION:
		{
			if (type1->u.function.paramNum != type2->u.function.paramNum) return false;
			FieldList param1 = type1->u.function.params,
					  param2 = type2->u.function.params;
			for (int i = 0; i < type1->u.function.paramNum; i++) {
				if (TypeEqual(param1->type, param2->type) == false)
					return false;
				param1 = param1->tail;
				param2 = param2->tail;
			}
			return true;
			break;
		}
		default: return false; break;
	}
}

void handle_Program(Node* root) {
	if (root == NULL) return;

	// Program -> ExtDefList
	if (root->childsum != 0)
		handle_ExtDefList(root->child[0]);
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
	Type basic = handle_Specifier(ExtDef->child[0]);

	// ExtDef -> Specifier ExtDecList SEMI
	if (strcmp(ExtDef->child[1]->name, "ExtDecList") == 0) {
		handle_ExtDecList(ExtDef->child[1], basic);
	}

	// ExtDef -> Specifier FunDec CompSt
	else if (strcmp(ExtDef->child[1]->name, "FunDec") == 0) {
		handle_FunDec(ExtDef->child[1], ExtDef, basic);
		handle_CompSt(ExtDef->child[2], basic);
	}

	// Specifier SEMI
	else {}
}

void handle_ExtDecList(Node* root, Type basic) {
	Node* ExtDecList = root;
	FieldList field;

	// ExtDecList -> VarDec COMMA ExtDecList
	while (ExtDecList->childsum == 3) {
		field = handle_VarDec(ExtDecList->child[0], basic);
		if (searchSymbol(field->name, variable_table) != NULL)
			printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", ExtDecList->lineno, field->name);
		else insertSymbol(field, variable_table);
		ExtDecList = ExtDecList->child[2];
	}

	// ExtDecList -> VarDec
	field = handle_VarDec(ExtDecList->child[0], basic);
	if (searchSymbol(field->name, variable_table) != NULL)
		printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", ExtDecList->lineno, field->name);
	else insertSymbol(field, variable_table);
}

Type handle_Specifier(Node* root) {
	Type Specifier = (Type)malloc(sizeof(Type_));
	
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
			FieldList field = searchSymbol(s, variable_table);
			if (field == NULL) {
				printf("Error type 17 at Line %d: Undefined structure \"%s\".\n", root->lineno, s);
				Specifier->u.structure = NULL;
				return Specifier;
			}
			else if (field->type != NULL)
				return field->type;
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
				Type basic = handle_Specifier(Def->child[0]);

				Node* DecList = Def->child[1];
				// DecList -> Dec COMMA DecList
				while (DecList->childsum == 3) {
					FieldList field = handle_VarDec(DecList->child[0]->child[0], Specifier);
					if (DecList->child[0]->childsum != 1)
						printf("Error type 15 at Line %d: Variable %s in struct is initialized.\n", Def->lineno, field->name);
					FieldList temp = Specifier->u.structure;
					while (temp != NULL) {
						if (strcmp(temp->name, field->name) == 0) {
							printf("Error type 15 at Line %d: Redefined field \"%s\".\n", Def->lineno, field->name);
							break;
						}
						temp = temp->tail;
					}

					if (temp == NULL) {
						if (searchSymbol(field->name, variable_table) != NULL)
							printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", Def->lineno, field->name);
						else {
							insertSymbol(field, variable_table);
							field->tail = Specifier->u.structure;
							Specifier->u.structure = field;
						}
					}
					DecList = DecList->child[2];
				}

				FieldList field = handle_VarDec(DecList->child[0]->child[0], basic);
				if (DecList->child[0]->childsum != 1)
					printf("Error type 15 at Line %d: Variable \"%s\" in struct is initialized.\n", Def->lineno, field->name);
				FieldList temp = Specifier->u.structure;
				while (temp != NULL) {
					if (strcmp(temp->name, field->name) == 0) {
						printf("Error type 15 at Line %d: Redefined field \"%s\".\n", Def->lineno, field->name);
						break;
					}
					temp = temp->tail;
				}
				if (temp == NULL) {
					if (searchSymbol(field->name, variable_table) != NULL)
						printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", Def->lineno, field->name);
					else {
						insertSymbol(field, variable_table);
						field->tail = Specifier->u.structure;
						Specifier->u.structure = field;
					}
				}
				DefList = DefList->child[1];
			}

			// OptTag -> ID
			if (StructSpecifier->child[1] != NULL) {
				FieldList field = (FieldList)malloc(sizeof(FieldList_));
				field->type = Specifier;
				field->name = StructSpecifier->child[1]->child[0]->text; // ID
				if (searchSymbol(field->name, variable_table) != NULL)
					printf("Error type 16 at Line %d: Duplicated name \"%s\".\n", root->lineno, field->name);
				else insertSymbol(field, variable_table);
			}

			return Specifier;
		}
	}
}

FieldList handle_VarDec(Node* root, Type basic) {
	Node* VarDec = root;
	// in case VarDec is an array
	// i remember the array's dimemsion
	int i = 0;
	while (strcmp(VarDec->child[0]->name, "ID") != 0) {
		VarDec = VarDec->child[0]; i++;
	}

	FieldList field = (FieldList)malloc(sizeof(FieldList_));
	field->name = VarDec->child[0]->text;

	// VarDec -> ID
	if (i == 0) {
		field->type = basic;
		return field;
	}

	Type var1 = (Type)malloc(sizeof(Type_));
	var1->kind = ARRAY;
	var1->u.array.size = atoi(root->child[2]->text);
	var1->u.array.elem = basic;

	switch(i) {
		case 1: 
			field->type = var1;
			return field;
		case 2: {
			Type var2 = (Type)malloc(sizeof(Type_));
			var2->kind = ARRAY;
			var2->u.array.size = atoi(root->child[0]->child[2]->text);
			var2->u.array.elem = var1;
			field->type = var2;
			return field;
		}
		default: printf("no more operation for array dimension above 2\n"); break;
	}
}

void handle_FunDec(Node* root, Node* ExtDef, Type basic) {
	FieldList field = (FieldList)malloc(sizeof(FieldList_));
	field->name = root->child[0]->text;
	Type type = (Type)malloc(sizeof(Type_));
	type->kind = FUNCTION;
	type->u.function.rtnType = basic;

	// FunDec -> ID LP RP
	type->u.function.params = NULL;
	type->u.function.paramNum = 0;

	// FunDec -> ID LP VarList RP
	if (strcmp(root->child[2]->name, "VarList") == 0) {
		Node* VarList = root->child[2];

		// VarList -> ParamDec COMMA VarList
		while (VarList->childsum == 3) {
			FieldList ParamDec_VarDec = handle_ParamDec(VarList->child[0], ExtDef);
			// ParamDec -> Specifier VarDec
/*			Type Specifier = handle_Specifier(VarList->child[0]->child[0]);
			FieldList VarDec = handleVarDec(VarList->child[0]->child[1], Specifier);
			if (searchSymbol(VarDec->name, variable_table) != NULL)
				printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", ExtDef->lineno, VarDec->name);
			else insertSymbol(VarDec, variable_table);*/
			type->u.function.paramNum++;
			ParamDec_VarDec->tail = type->u.function.params;
//			VarDec->tail = type->u.function.params;
			type->u.function.params = ParamDec_VarDec;

			VarList = VarList->child[2];
		}

		// VarList -> ParamDec
		FieldList ParamDec_VarDec = handle_ParamDec(VarList->child[0], ExtDef);	
		type->u.function.paramNum++;
		ParamDec_VarDec->tail = type->u.function.params;
		type->u.function.params = ParamDec_VarDec;
	}

	field->type = type;
	if (searchSymbol(field->name, function_table) != NULL)
		printf("Error type 4 at Line %d: Redefined function \"%s\".\n", ExtDef->lineno, field->name);
	else insertSymbol(field, function_table);
}

FieldList handle_ParamDec(Node* root, Node* ExtDef) {
	// ParamDec -> Specifier VarDec
	Type Specifier = handle_Specifier(root->child[0]);
	FieldList VarDec = handle_VarDec(root->child[1], Specifier);
	if (searchSymbol(VarDec->name, variable_table) != NULL)
		printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", ExtDef->lineno, VarDec->name);
	else insertSymbol(VarDec, variable_table);

	return VarDec;
}

void handle_CompSt(Node* root, Type rtnType) {
	Node* CompSt = root;

	// CompSt -> LC DefList StmtList RC
	handle_DefList(CompSt->child[1]);
	handle_StmtList(CompSt->child[2], rtnType);
}

void handle_StmtList(Node* root, Type rtnType) {
	Node* StmtList = root;

	// StmtList -> Stmt StmtList
	while (StmtList != NULL) {
		handle_Stmt(StmtList->child[0], rtnType);
		StmtList = StmtList->child[1];
	}
}

void handle_Stmt(Node* root, Type rtnType) {
	Node* Stmt = root;

	// Stmt -> Exp SEMI
	if (strcmp(Stmt->child[0]->name, "Exp") == 0)
		handle_Exp(Stmt->child[0]);

	// Stmt -> CompSt
	else if (strcmp(Stmt->child[0]->name, "CompSt") == 0)
		handle_CompSt(Stmt->child[0], rtnType);
	
	// Stmt -> RETURN Exp SEMI
	else if (strcmp(Stmt->child[0]->name, "RETURN") == 0) {
		Type tempRtnType = handle_Exp(Stmt->child[1]);
		if (TypeEqual(rtnType, tempRtnType) == false)
			printf("Error type 8 at Line %d: Type mismatched for return.\n", Stmt->lineno);
	}

	// Stmt -> WHILE LP Exp RP Stmt
	else if (strcmp(Stmt->child[0]->name, "WHILE") == 0) {
		Type type = handle_Exp(Stmt->child[2]);
		if (!((type->kind == BASIC) && (type->u.basic.type == INT)))
			printf("Error type 5 at Line %d: Only type INT could be used for judgement.\n", Stmt->lineno);
		handle_Stmt(Stmt->child[4], rtnType);
	}

	// Stmt -> IF LP Exp RP Stmt
	else if (Stmt->childsum == 5) {
		Type type = handle_Exp(Stmt->child[2]);
		if (type != NULL)
			if (!((type->kind == BASIC) && (type->u.basic.type == INT)))
				printf("Error type 5 at Line %d: Only type INT could be used for judgement.\n", Stmt->lineno);

		handle_Stmt(Stmt->child[4], rtnType);
	}

	// Stmt -> IF LP Exp RP Stmt ELSE Stmt 
	else {
		Type type = handle_Exp(Stmt->child[2]);
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
	Type basic = handle_Specifier(Def->child[0]);
	handle_DecList(Def->child[1], basic);
}

void handle_DecList(Node* root, Type basic) {
	Node* DecList = root;

	// DecList -> Dec COMMA DecList
	while (DecList->childsum == 3) {
		handle_Dec(DecList->child[0], DecList, basic);
		DecList = DecList->child[2];
	}
	handle_Dec(DecList->child[0], DecList, basic);
}

void handle_Dec(Node* root, Node* DecList, Type basic) {
	Node* Dec = root;

	// Dec -> VarDec
	FieldList field = handle_VarDec(root->child[0], basic);
	if (searchSymbol(field->name, variable_table) != NULL)
		printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", DecList->lineno, field->name);
	else insertSymbol(field, variable_table);

	/* TODO */
}

Type handle_Exp(Node* root) {
	if (root == NULL) return NULL;

	// handle orderd by paramsNum

	// Exp -> ID
	if ((strcmp(root->child[0]->name, "ID") == 0) && (root->childsum == 1)) {
		FieldList field = searchSymbol(root->child[0]->text, variable_table);
		if (field != NULL) return field->type;
		else {
			printf("Error type 1 at Line %d: Undefined variable \"%s\".\n", root->lineno, root->child[0]->text);
			return NULL;
		}
	}

	// Exp -> INT
	else if (strcmp(root->child[0]->name, "INT") == 0) {
		Type type = (Type)malloc(sizeof(Type_));
		type->kind = BASIC;
		type->u.basic.type = INT;
		type->u.basic.name = root->child[0]->text;
		return type;
	}

	// Exp -> FLOAT
	else if (strcmp(root->child[0]->name, "FLOAT") == 0) {
		Type type = (Type)malloc(sizeof(Type_));
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
		Type left = handle_Exp(root->child[0]),
			 right = handle_Exp(root->child[2]);
		if (TypeEqual(left, right) == false) {
			if ((left != NULL) && (right != NULL))
				printf("Error type 5 at Line %d: Type mismatched for assignment.\n", root->lineno);
			return NULL;
		}
		else return left;
	}
	
	// Exp -> Exp AND Exp
	// Exp -> Exp OR Exp
	// Exp -> Exp RELOP Exp
	else if ((strcmp(root->child[1]->name, "AND") == 0) || (strcmp(root->child[1]->name, "OR") == 0) || (strcmp(root->child[1]->name, "RELOP") == 0)) {
		Type left = handle_Exp(root->child[0]),
			 right = handle_Exp(root->child[2]);
		if (TypeEqual(left, right) == false) {
			if ((left != NULL) && (right != NULL))
				printf("Error type 7 at Line %d: Type mismatched for operands.\n", root->lineno);
			return NULL;
		}
		else {
			Type type = (Type)malloc(sizeof(Type_));
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
		Type left = handle_Exp(root->child[0]),
			 right = handle_Exp(root->child[2]);
		if (TypeEqual(left, right) == false) {
			if ((left != NULL) && (right != NULL))
				printf("Error type 7 at Line %d: Type mismatched for operands.\n", root->lineno);
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
		FieldList field = searchSymbol(root->child[0]->text, function_table);
		if (field == NULL) {
			FieldList f = searchSymbol(root->child[0]->text, variable_table);
			if (f != NULL)
				printf("Error type 11 at Line %d: \"%s\" is not a function.\n", root->lineno, root->child[0]->text);
			else printf("Error type 2 at Line %d: Undefined function \"%s\".\n", root->lineno, root->child[0]->text);
			return NULL;
		}

		Type funcType = field->type;
		Type type = (Type)malloc(sizeof(Type_));
		type->kind = FUNCTION;
		type->u.function.paramNum = 0;
		type->u.function.params = NULL;

		// Exp -> ID LP Args RP
		if (strcmp(root->child[2]->name, "Args") == 0)
			handle_Args(root->child[2], &type);

		if (TypeEqual(type, funcType) == false) {
			printf("Error type 9 at Line %d: Params mismatched in function \"%s\".\n", root->lineno, root->child[0]->text);
			return NULL;
		}

		else return funcType->u.function.rtnType;
	}

	// Exp -> Exp LB Exp RB
	else if (strcmp(root->child[1]->name, "LB") == 0) {
		Type Exp = handle_Exp(root->child[0]);
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

		Type Exp2 = handle_Exp(root->child[2]);
		if (Exp2->kind != BASIC || (Exp2->kind == BASIC && Exp2->u.basic.type != INT)) {
			printf("Error type 12 at Line %d: \"%s\" is not an integer.\n", root->lineno, Exp2->u.basic.name);
			return NULL;
		}

		return Exp->u.array.elem;
	}

	// Exp -> Exp DOT ID
	else if (strcmp(root->child[1]->name, "DOT") == 0) {
		Type Exp = handle_Exp(root->child[0]);
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
			if (strcmp(temp->name, s) == 0) return temp->type;
			temp = temp->tail;
		}

		printf("Error type 14 at Line %d: Non-existent field \"%s\".\n", root->lineno, root->child[2]->text);
		return NULL;
	}

	// no error!
	else return NULL;
}

void handle_Args(Node* root, Type* type) {
	Node* Args = root;

	// Args -> Exp COMMA Args
	while (Args->childsum != 1) {
		Type Exp = handle_Exp(Args->child[0]);
		FieldList field = (FieldList)malloc(sizeof(FieldList_));
		field->type = Exp;
		field->tail = (*type)->u.function.params;
		(*type)->u.function.paramNum++;
		(*type)->u.function.params = field;

		Args = Args->child[2];
	}

	// Args -> Exp
	Type Exp = handle_Exp(Args->child[0]);
	FieldList field = (FieldList)malloc(sizeof(FieldList_));
	field->type = Exp;
	field->tail = (*type)->u.function.params;
	(*type)->u.function.paramNum++;
	(*type)->u.function.params = field;
}
