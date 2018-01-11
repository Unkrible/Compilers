#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "SyntaxTree.h"
#include "InterCode.h"

#define BINGO 0
#define ERROR_REDEFINE 1
#define ERROR_DECLARATION_CONFLICT 2
#define ERROR_DOMAIN_REDEFINE 3

#define FUNC_IS_DECLARED 4

#define FROM_VARIABLE 1
#define FROM_FIELD 2
#define FROM_PARAM 3

typedef struct Type_* Type;
typedef struct Structure_* Structure;
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
		Structure structure;

		// function information
		Function function;
	} u;
	enum { LEFT,RIGHT,BOTH } assign;
};

struct Structure_{
	char *name;
	FieldList domain;
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
int typeSize(Type type);
int structEqual(Structure lhs, Structure rhs);
int valueEqual(FieldList lhs, FieldList rhs);

void Program(Node* root);
void ExtDefList(Node *n);
void ExtDef(Node *n);
void ExtDecList(Node *n, Type type);

Type Specifier(Node *n);
Type StructSpecifier(Node *n);

FieldList VarDec(Node *n, Type type, int from);
Function FunDec(Node *n, Type type);
FieldList VarList(Node *n);
FieldList ParamDec(Node *n);

void CompSt(Node *n, Type retype);
void StmtList(Node *n, Type retype);
void Stmt(Node *n, Type retype);

FieldList DefList(Node *n, int from);
FieldList Def(Node *n, int from);
FieldList DecList(Node *n, Type type, int from);
FieldList Dec(Node *n, Type type, int from);

Type Exp(Node *n, Operand place);
Type Exp_Cond(Node *n,Operand label_true,Operand label_false);
int Args(Node *n, FieldList param, Operand arg);

#endif
