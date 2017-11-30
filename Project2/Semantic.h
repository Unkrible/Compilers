#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "SyntaxTree.h"
#include "HashTable.h"

#define ERROR_REDEFINE 1
#define ERROR_DECLARATION_CONFLICT 2

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
	int line;
	Type retype;
	FieldList param;
};

int typeEqual(Type lhs, Type rhs);

void Program(Node* root);
void ExtDefList(Node *n);
void ExtDefList(Node *n, Type type);
void ExtDef(Node *n);

Type Specifier(Node *n);
Type StructSpecifier(Node *n);

FieldList VarDec(Node *n, Type type);
Function FunDec(Node *n, Type type);
FieldList VarList(Node *n, FieldList varList);
FieldList ParamDec(Node *n);

void CompSt(Node *n, Type retype);
void StmtList(Node *n, Type retype);
void Stmt(Node *n, Type retype);

FieldList DefList(Node *n);
FieldList Def(Node *n, FieldList defList);
FieldList DecList(Node *n, Type type);
FieldList Dec(Node *n, Type type);

Type Exp(Node *n);

#endif
