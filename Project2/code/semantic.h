#ifndef _SEMANTIC_H_
#define _SEMANTIC_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node.h"

typedef struct Type_ *Type;
typedef struct FieldList_ *FieldList;

struct Type_ {
	enum { BASIC, ARRAY, STRUCTURE } kind;

	union {
		// basic type(INT or FLOAT)
		int basic;

		// array type
		struct {
			Type elem; int size;
		} array;

		// structure type itself is a list
		FieldList structure;
	} u;
};

struct FieldList_ {
	char* name;
	Type type;
	FieldList tail;
};

#endif
