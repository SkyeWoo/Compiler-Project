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
			Specifier->u.basic = INT;
		else Specifier->u.basic = FLOAT;
		return Specifier;
	}
	/* TODO Specifier -> Structure */
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

void handle_CompSt(Node* root, Type basic) {
	Node* CompSt = root;

	// CompSt -> LC DefList StmtList RC
	handle_DefList(CompSt->child[1]);
	handle_StmtList(CompSt->child[2], basic);
}

void handle_StmtList(Node* root, Type basic) {
	Node* StmtList = root;

	// StmtList -> Stmt StmtList
	while (StmtList != NULL) {
		handle_Stmt(StmtList->child[0], basic);
		StmtList = StmtList->child[1];
	}
}

void handle_Stmt(Node* root, Type basic) {
	Node* Stmt = root;

	// Stmt -> Exp SEMI
	if (strcmp(Stmt->child[0]->name, "Exp") == 0)
		handle_Exp(Stmt->child[0]);

	// Stmt -> CompSt
	else if (strcmp(Stmt->child[0]->name, "CompSt") == 0)
		handle_CompSt(Stmt->child[0], basic);
	
	// Stmt -> RETURN Exp SEMI
	else if (strcmp(Stmt->child[0]->name, "RETURN") == 0) {
		/* TODO error */
	}

	// Stmt -> WHILE LP Exp RP Stmt
	else if (strcmp(Stmt->child[0]->name, "WHILE") == 0) {
			/* TODO */
	}

	// Stmt -> IF LP Exp RP Stmt
	else if (Stmt->childsum == 5) {
			/* TODO */
	}

	// Stmt -> IF LP Exp RP Stmt ELSE Stmt 
	else {
			/* TODO */
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
	return NULL;
}
