#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "SyntaxTree.h"

typedef struct Type_* Type;
typedef struct FieldList_* FieldList;
typedef struct Function_* Function;

struct Type_{
	enum { BASIC,ARRAY,STRUCTURE,FUNCTION } kind;
	union{
		// basic type
		int basic;
		
		// array information
		struct { Type elem; int size;} array;
		
		// structure information
		FieldList structure;	
		
		// function information
		Function function;
	} u;
};

struct FieldList_{
	// Field name
	char *name;
	// Field type
	Type type;
	// next field
	FieldList tail;
};

struct Function_{
	char *name;
	int isDefined;
	int row;
	Type retype;
	FieldList param;
	int num;
};

void Program(Node* root);
void ExtDefList(Node *n);
void ExtDefList(Node *n, Type type);
void ExtDef(Node *n);

Type Specifier(Node *n);

void VarDec(Node *n, Type type);

#endif
