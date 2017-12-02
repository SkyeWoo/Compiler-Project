%{
	#include <stdio.h>
	#include <stdlib.h>
	#include "node.h"
	#include "lex.yy.c"

	extern int yylineno;
	extern Node* root;
	extern int errorNum;
	extern void myerror(char *msg);
	extern void myerror(char *msg);
	extern void yyerror(char *msg);
%}

%union {
	struct TreeNode* node;
}

%token <node> INT FLOAT ID SEMI COMMA ASSIGNOP RELOP PLUS MINUS STAR DIV AND OR DOT NOT TYPE LP RP LB RB LC RC STRUCT RETURN IF ELSE WHILE

%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT
%left DOT LB RB LP RP

%nonassoc MISSING_RC MISSING_RB MISSING_RP MISSING_SEMI
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%type <node> Program ExtDefList ExtDef ExtDecList Specifier
%type <node> StructSpecifier OptTag Tag VarDec FunDec VarList
%type <node> ParamDec CompSt StmtList Stmt DefList Def DecList
%type <node> Dec Exp Args


%%


// High-level Definitions

Program		: ExtDefList { $$ = createNode("Program", ""); addChild(1, $$, $1); root = $$; }
		 	;

ExtDefList	: ExtDef ExtDefList { $$ = createNode("ExtDefList", ""); addChild(2, $$, $1, $2); }
		   	| { $$ = NULL; }
			;

ExtDef		: Specifier ExtDecList SEMI { $$ = createNode("ExtDef", ""); addChild(3, $$, $1, $2, $3); }
			| Specifier SEMI { $$ = createNode("ExtDef", ""); addChild(2, $$, $1, $2); }
//			| Specifier error SEMI { myerror("1Syntax error"); }
			| Specifier FunDec CompSt { $$ = createNode("ExtDef", ""); addChild(3, $$, $1, $2, $3); }
// REQUIRE 1 function declaration
			| Specifier FunDec SEMI { $$ = createNode("ExtDef", ""); addChild(3, $$, $1, $2, $3); /* myerror("Syntax error"); */ }
			| Specifier error { myerror("1Syntax error"); }
			;

ExtDecList	: VarDec { $$ = createNode("ExtDecList", ""); addChild(1, $$, $1); }
		   	| VarDec COMMA ExtDecList { $$ = createNode("ExtDecList", ""); addChild(3, $$, $1, $2, $3); }
			;

// Specifiers

Specifier	: TYPE { $$ = createNode("Specifier", $1->text); addChild(1, $$, $1); }
		  	| StructSpecifier { $$ = createNode("Specifier", ""); addChild(1, $$, $1); }
			;

StructSpecifier : STRUCT OptTag LC DefList RC { $$ = createNode("StructSpecifier", ""); addChild(5, $$, $1, $2, $3, $4, $5); }
		/*		| STRUCT OptTag LC error RC { myerror("Missing \"}\""); }*/
				| STRUCT Tag { $$ = createNode("StructSpecifier", ""); addChild(2, $$, $1, $2); }
				;

OptTag		: ID { $$ = createNode("OptTag", $1->text); addChild(1, $$, $1); }
			| { $$ = NULL; }
			;

Tag			: ID { $$ = createNode("Tag", $1->text); addChild(1, $$, $1); }
	  		;

// Declarators

VarDec		: ID { $$ = createNode("VarDec", $1->text); addChild(1, $$, $1); }
			| VarDec LB INT RB { $$ = createNode("VarDec", ""); addChild(4, $$, $1, $2, $3, $4); }
			| VarDec LB error RB { myerror("Missing \"]\""); }
			;

FunDec		: ID LP VarList RP { $$ = createNode("FunDec", ""); addChild(4, $$, $1, $2, $3, $4); }
/*			| ID LP error RP { myerror("Syntax error"); }*/
			| ID LP RP { $$ = createNode("FunDec", ""); addChild(3, $$, $1, $2, $3); }
//			| error RP { myerror("2Syntax error"); }
			;

VarList		: ParamDec COMMA VarList { $$ = createNode("VarList", ""); addChild(3, $$, $1, $2, $3); }
		 	| ParamDec { $$ = createNode("VarList", ""); addChild(1, $$, $1); }
			;

ParamDec	: Specifier VarDec { $$ = createNode("ParamDec", ""); addChild(2, $$, $1, $2); }
//			| error COMMA { myerror("3Syntax error"); }
			;

// Statements

CompSt		: LC DefList StmtList RC { $$ = createNode("CompSt", ""); addChild(4, $$, $1, $2, $3, $4); }
			| LC error RC { myerror("5Syntax error"); }
			| LC DefList StmtList %prec MISSING_RC { myerror("Missing \"}\""); }
			;

StmtList	: Stmt StmtList { $$ = createNode("StmtList", ""); addChild(2, $$, $1, $2); }
		 	| { $$ = NULL; }
			;

Stmt		: Exp SEMI { $$ = createNode("Stmt", ""); addChild(2, $$, $1, $2); }
	  		| Exp error SEMI { myerror("Syntax error"); }
	  		| CompSt { $$ = createNode("Stmt", ""); addChild(1, $$, $1); }
			| RETURN Exp SEMI { $$ = createNode("Stmt", ""); addChild(3, $$, $1, $2, $3); }
			| IF LP Exp RP Stmt %prec LOWER_THAN_ELSE { $$ = createNode("Stmt", ""); addChild(5, $$, $1, $2, $3, $4, $5); }
/*			| IF LP error RP Stmt %prec LOWER_THAN_ELSE { myerror("Missing \")\""); }*/
			| IF LP Exp RP Stmt ELSE Stmt { $$ = createNode("Stmt", ""); addChild(7, $$, $1, $2, $3, $4, $5, $6, $7); }
/*			| IF LP error RP Stmt ELSE Stmt { myerror("Missing \")\""); }*/
			| IF LP Exp RP Exp error ELSE Stmt { myerror("Missing \";\""); }
			| WHILE LP Exp RP Stmt { $$ = createNode("Stmt", ""); addChild(5, $$, $1, $2, $3, $4, $5); }
			| WHILE LP error RP Stmt { myerror("Syntax error"); }
			;

// Local Definitions

DefList		: Def DefList { $$ = createNode("DefList", ""); addChild(2, $$, $1, $2); }
		 	| { $$ = NULL; }
			;

Def			: Specifier DecList SEMI { $$ = createNode("Def", ""); addChild(3, $$, $1, $2, $3); }
	  		| Specifier DecList %prec MISSING_SEMI { myerror("Missing \";\""); }
	  		| error SEMI { myerror("6Syntax error"); }
	  		;

DecList		: Dec { $$ = createNode("DecList", ""); addChild(1, $$, $1); }
		 	| Dec COMMA DecList { $$ = createNode("DecList", ""); addChild(3, $$, $1, $2, $3); }
			;

Dec			: VarDec { $$ = createNode("Dec", ""); addChild(1, $$, $1); }
	  		| VarDec ASSIGNOP Exp { $$ = createNode("Dec", ""); addChild(3, $$, $1, $2, $3); }
			;

// Expressions

Exp			: Exp ASSIGNOP Exp { $$ = createNode("Exp", ""); addChild(3, $$, $1, $2, $3); }
	  		| Exp AND Exp { $$ = createNode("Exp", ""); addChild(3, $$, $1, $2, $3); }
			| Exp OR Exp { $$ = createNode("Exp", ""); addChild(3, $$, $1, $2, $3); }
			| Exp RELOP Exp { $$ = createNode("Exp", ""); addChild(3, $$, $1, $2, $3); }
			| Exp PLUS Exp { $$ = createNode("Exp", ""); addChild(3, $$, $1, $2, $3); }
			| Exp MINUS Exp { $$ = createNode("Exp", ""); addChild(3, $$, $1, $2, $3); }
			| Exp STAR Exp { $$ = createNode("Exp", ""); addChild(3, $$, $1, $2, $3); }
			| Exp DIV Exp { $$ = createNode("Exp", ""); addChild(3, $$, $1, $2, $3); }
			| LP Exp RP { $$ = createNode("Exp", ""); addChild(3, $$, $1, $2, $3); }
			| LP error RP { myerror("7Syntax error"); }
			| MINUS Exp { $$ = createNode("Exp", ""); addChild(2, $$, $1, $2); }
			| NOT Exp { $$ = createNode("Exp", ""); addChild(2, $$, $1, $2); }
			| ID LP Args RP { $$ = createNode("Exp", ""); addChild(4, $$, $1, $2, $3, $4); }
/*			| ID LP error RP { myerror("Syntax error"); } */
			| ID LP RP { $$ = createNode("Exp", ""); addChild(3, $$, $1, $2, $3); }
			| Exp LB Exp RB { $$ = createNode("Exp", ""); addChild(4, $$, $1, $2, $3, $4); }
			| Exp LB error RB { myerror("Missing \"]\""); /*myerror("8Syntax error");*/ }
			| Exp DOT ID { $$ = createNode("Exp", ""); addChild(3, $$, $1, $2, $3); }
			| ID { $$ = createNode("Exp", ""); addChild(1, $$, $1); }
			| INT { $$ = createNode("Exp", ""); addChild(1, $$, $1); }
			| FLOAT { $$ = createNode("Exp", ""); addChild(1, $$, $1); }
			;

Args		: Exp COMMA Args { $$ = createNode("Args", ""); addChild(3, $$, $1, $2, $3); }
	  		| Exp { $$ = createNode("Args", ""); addChild(1, $$, $1); }
			;


%%
void myerror(char *msg) {
	errorNum++;
	fprintf(stderr, "Error type B at Line %d: %s.\n", yylineno, msg);
}

void yyerror(char *msg) {
}

/*int main(int argc, char **argv) {
	if (argc <= 1) return 1;

	FILE *fp = fopen(argv[1], "r");
	if (!fp) {
		perror(argv[1]); return 1;
	}

	yylineno = 1;
	yyrestart(fp);
	yyparse();

	if (errorNum == 0) printTree(root, 0);

	return 0;
}*/
