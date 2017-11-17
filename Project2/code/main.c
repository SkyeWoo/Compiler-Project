#include <stdio.h>
#include <stdlib.h>
#include "node.h"
#include "semantic.h"

Node* root = NULL;
int errorNum = 0;

extern void yyrestart(FILE*);
extern int yyparse();
extern int yylineno;

int main(int argc, char** argv) {
	if (argc <= 1) return 1;

	FILE *fp = fopen(argv[1], "r");
	if (!fp) {
		perror(argv[1]); return 1;
	}

	yylineno = 1;
	yyrestart(fp);
	yyparse();

	if (errorNum == 0) {
		//printTree(root, 0);
		initHashTable();
	}

	return 0;
}
