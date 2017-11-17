%{
	#include <stdio.h>
	#include <stdlib.h>
	#include <ctype.h>
	#include <string.h>
	#include "syntax.tab.h"
	#include "node.h"

	extern int errorNum;

/*	int yycolumn = 1;
#define YY_USER_ACTION
	yylloc.first_line = yylloc.last_line = yylineno;
	yylloc.first_column = yycolumn;
	yylloc.last_column = yycolumn + yyleng - 1;
	yycolumn += yyleng;
	*/
%}

%option yylineno

%x COMMENT

digit [0-9]
FLOAT [+-]?(({digit}*[.]{digit}+)|({digit}+[.]{digit}*))([Ee][+-]?{digit}+)?f?
FLOAT_ERROR [+-]?((({digit}*[.]{digit}*)?[Ee][+-]?{FLOAT}?)|([.][Ee][+-]?{digit}*))

DECNUM ([1-9][0-9]*)|0
OCT 0[0-7]*
HEX 0[Xx][0-9a-fA-F]*
OCT_ERROR 0[0-9A-WYZa-wyz][0-9A-Za-z]*
HEX_ERROR 0[Xx][0-9A-Za-z]*
INT {DECNUM}|{OCT}|{HEX}

ID [A-Za-z_][A-Za-z_0-9]*

SEMI ;
COMMA ,
ASSIGNOP =
RELOP >|<|>=|<=|==|!=
PLUS "+"
MINUS "-"
STAR "*"
DIV "/"
AND "&&"
OR "||"
DOT "."
NOT "!"
TYPE int|float
LP \(
RP \)
LB \[
RB \]
LC \{
RC \}
STRUCT struct
RETURN return
IF if
ELSE else
WHILE while

SPACE [ \r\n\t]+
OTHER .


%%


{SEMI} {
	yylval.node = createNode("SEMI", yytext); return(SEMI);
}

{COMMA} {
	yylval.node = createNode("COMMA", yytext); return(COMMA);
}

{ASSIGNOP} {
	yylval.node = createNode("ASSIGNOP", yytext); return(ASSIGNOP);
}

{RELOP} {
	yylval.node = createNode("RELOP", yytext); return(RELOP);
}

{PLUS} {
	yylval.node = createNode("PLUS", yytext); return(PLUS);
}

{MINUS} {
	yylval.node = createNode("MINUS", yytext); return(MINUS);
}

{STAR} {
	yylval.node = createNode("STAR", yytext); return(STAR);
}

{DIV} {
	yylval.node = createNode("DIV", yytext); return(DIV);
}

{AND} { 
	yylval.node = createNode("AND", yytext); return(AND); 
} 

{OR} { 
	yylval.node = createNode("OR", yytext); return(OR); 
} 

{DOT} { 
	yylval.node = createNode("DOT", yytext); return(DOT); 
} 

{NOT} { 
	yylval.node = createNode("NOT", yytext); return(NOT); 
} 

{TYPE} { 
	yylval.node = createNode("TYPE", yytext); return(TYPE); 
} 

{LP} { 
	yylval.node = createNode("LP", yytext); return(LP); 
} 

{RP} { 
	yylval.node = createNode("RP", yytext); return(RP); 
} 

{LB} { 
	yylval.node = createNode("LB", yytext); return(LB); 
} 

{RB} { 
	yylval.node = createNode("RB", yytext); return(RB); 
} 

{LC} { 
	yylval.node = createNode("LC", yytext); return(LC); 
} 

{RC} { 
	yylval.node = createNode("RC", yytext); return(RC); 
} 

{STRUCT} { 
	yylval.node = createNode("STRUCT", yytext); return(STRUCT); 
} 

{RETURN} { 
	yylval.node = createNode("RETURN", yytext); return(RETURN); 
}

{IF} {
	yylval.node = createNode("IF", yytext); return(IF);
}

{ELSE} { 
	yylval.node = createNode("ELSE", yytext); return(ELSE);
}

{WHILE} {
	yylval.node = createNode("WHILE", yytext); return(WHILE);
}

{SPACE} {}

{OCT} {
	char *endptr = NULL;
	int tol = (int)strtol(yytext, &endptr, 8);
	snprintf(yytext, 10, "%d", tol);
	yylval.node = createNode("INT", yytext);
	return(INT);
}

{HEX} {
	char *endptr = NULL;
	int tol = (int)strtol(yytext, &endptr, 16);
	snprintf(yytext, 10, "%d", tol);
	yylval.node = createNode("INT", yytext);
	return(INT);
}

{OCT_ERROR} {
	errorNum++;
	printf("Error type A at line %d: Illegal octal number '%s'.\n", yylineno, yytext);
	return(INT);
}

{HEX_ERROR} {
	errorNum++;
	printf("Error type A at line %d: Illegal hexadecimal number '%s'.\n", yylineno, yytext);
	return(INT);
}

{INT} {
	yylval.node = createNode("INT", yytext); return(INT);
}

{FLOAT} {
	yylval.node = createNode("FLOAT", yytext); return(FLOAT);
}

{FLOAT_ERROR} {
	errorNum++;
	printf("Error type A at line %d: Illegal floating point number '%s'.\n", yylineno, yytext);
	return(FLOAT);
}

{ID} {
	yylval.node = createNode("ID", yytext); return(ID);
}

{OTHER} {
	errorNum++;
	printf("Error type A at line %d: Mysterious character \"%s\".\n", yylineno, yytext);
}

"//".* {
	char c = input();
	while (c != '\n') c = input();
}

"/*" { BEGIN COMMENT; }
<COMMENT>[^*\n]* {}
<COMMENT>"*"+[^*/\n]* {}
<*>\n {}
<COMMENT>"*/" { BEGIN INITIAL;}
<COMMENT><<EOF>> {
	errorNum++;
	printf("Error type A at line %d: met EOF\n", yylineno);
	BEGIN INITIAL;
}

"*/" {
	errorNum++;
	printf("Error type B at line %d: Syntax error.\n", yylineno);
}


%%

int yywrap() {
	return 1;
}
