#include "node.h"

Node* createNode(char *name, char* text) {
	Node* p = (Node*)malloc(sizeof(Node));

	strcpy(p->name, name);
	strcpy(p->text, text);
	
	p->lineno = yylineno;
	int i = 0;
	for (; i < MAX_CHILD_NUM; i++)
		p->child[i] = NULL;
	
	p->childsum = 0;
	
	return p;
}

void addChild(int childsum, Node* parent, ...) {
	va_list ap;
	va_start(ap, parent);
	
	int i = 0;
	for (; i < childsum; i++)
		parent->child[i] = va_arg(ap, Node*);

	parent->lineno = parent->child[0]->lineno;
	parent->childsum = childsum;
	va_end(ap);
}

void printTree(Node* parent, int blank) {
	if (parent == NULL) return;
	
	int i = 0;
	for(; i < blank; i++) printf(" ");
	
	if (parent->childsum != 0) {
		printf("%s (%d)\n", parent->name, parent->lineno);
		for (i = 0; i < parent->childsum; i++)
			printTree(parent->child[i], blank + 2);
	}
	else {
		if (strcmp(parent->name, "ID") == 0 || strcmp(parent->name, "TYPE") == 0)
			printf("%s: %s\n", parent->name, parent->text);
		else if (strcmp(parent->name, "INT") == 0)
			printf("%s: %d\n", parent->name, atoi(parent->text));
		else if (strcmp(parent->name, "FLOAT") == 0)
			printf("%s %f\n", parent->name, atof(parent->text));
		else printf("%s\n", parent->name);
	}
}
