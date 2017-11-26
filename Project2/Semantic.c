#include "Semantic.h"
#include "SyntaxTree.h"
#include "HashTable.h"
#include <string.h>

/* High-level Definitions */
void Program(Node* root){
	// ExtDefList
	ExtDefList(root->child);
}

void ExtDefList(Node* n){
	// ExtDefList -> NULL
	if(n->child == NULL)
		return;
	// ExtDef ExtDefList
	ExtDef(n->child);
	ExtDefList(n->child->sibling);
}

/* Global Variable */
void ExtDef(Node *n){
	Node* child = n->child;
	Type type;
	type = Specifier(child);
	child = child->sibling;

	// Specifier SEMI
	if(strcmp(child->identifier,"SEMI")==0)
			return;
	// Specifier ExtDecList SEMI
	else if(strcmp(child->identifier,"ExtDecList")==0)
			ExtDecList(child,type);
	// Specifier FunDec CompSt
	else if(strcmp(child->identifier,"FunDec")==0){
		Function func=FunDec(child,type);
	
		//TODO: Complete Func insert
		child = child->sibling;
		int i;
		if(strcmp(child->identifier,"CompSt")==0){
			if(func==NULL)
					return;
			func->isDefined = 1;
			int flag=funcInsertCheck(func);
			insertTable(func);
			CompSt(child,type);
		}
	}
	else
			return;
}

void ExtDecList(Node *n, Type type){
	Node *child = n->child;
	if(child == NULL)
			return;
	VarDec(child, type);
	
	// VarDec COMMA ExtDecList
	child = child->sibling;
	if(child == NULL)
			return;
	child = child->sibling;
	if(child == NULL)
			return;
	ExtDecList(child, type);
}

/* Specifiers */
 
Type Specifier(Node *n){
	Node *child = n->child;
	if(child == NULL)
			exit(-1);

	Type type;

	// TYPE
	if(strcmp(child->identifier,"TYPE")==0){
		type = (Type)malloc(sizeof(struct Type_));
		type->kind = BASIC;
		if(strcmp(child->value,"int")==0){
			type->u.basic = TYPE_INT;
		}
		else if(strcmp(child->value, "float")==0){
			type->u.basic = TYPE_FLOAT;
		}
		else
				exit(-1);
	}
	// StructSpecifier
	else if(strcmp(child->identifier, "StructSpecifier")==0){
		type = StructSpecifier(child);
	}
	else
			exit(-1);

	return type;
}

Type StructSpecifier(Node *n){
	Node *child = n->child;
	if(child == NULL)
			exit(-1);

	Type type = (Type)malloc(sizeof(struct Type_));
	type->kind = STRUCTURE;
	FieldList fl = (FieldList)malloc(sizeof(struct FieldList_));
	fl->type = type;

	child = child->sibling;
	
	// STRUCT OptTag LC DefList RC
	if(strcmp(child->identifier, "OptTag")==0){
		// OptTag -> NULL
		if(child->child == NULL){
			fl->name = NULL;
		}
		// OptTag -> ID
		else{
			fl->name = child->child->value;
		}
		child = child->sibling->sibling;
		fl->tail = DefList(child);
	}
	// STRUCT Tag
	else if(strcmp(child->identifier, "Tag")==0){
		fl->name = child->child->value;
		fl->tail = NULL;	
	}
	else
			exit(-1);

	return type;
}

/* Declarators */
FieldList VarDec(Node *n, Type type){
	Node *child = n->child;
	FieldList varDec=NULL;
	if(child == NULL)
			return;

	// ID
	if(strcmp(child->identifier, "ID")==0){
		varDec = (FieldList)malloc(sizeof(struct FieldList_));
		varDec->name = child->value;
		varDec->type = type;
		varDec->tail = NULL;
	}
	// VarDec LB INT RB
	else if(strcmp(child->identifier, "VarDec")==0){
		varDec = VarDec(child, type);
		if(varDec == NULL)	
				return NULL;

		child = child->sibling;
		child = child->sibling;
		Type tmpType = varDec->type;
		Type newType = (Type)malloc(sizeof(struct Type_));
		newType->kind = ARRAY;
		newType->u.array.size = (int)strtol(child->value,NULL,10);
		
		if(type->kind==BASIC || type->kind==STRUCTURE){
			newType->u.array.elem = type;
			varDec->type = newType;	
		}
		else if(type->kind == ARRAY){
			while((varDec->type->u.array.elem)->kind == ARRAY)
					varDec->type = var->type->u.array.elem;
			newType->u.arrary.elem = varDec->type;
			varDec->type = newType;
		}
	}
	else
		exit(-1);

	return varDec;
}

Function FunDec(Node *n, Type type){
	Node *child = n->child;
	Function func = (Function)malloc(sizeof(struct Function_));
	func->name = child->value;
	func->isDefined = 0;
	func->param = NULL;
	func->retype = type;
	func->line = child->line;

	child = child->sibling->sibling;
	// ID LP VarList RP
	if(strcmp(child->identifier,"VarList")==0){
		FieldList varList = (FieldList)malloc(sizeof(struct FieldList_));
		varList->name = func->name;
		varList->type = NULL;
		varList->tail = NULL:
		func->param = VarList(child, varList);
	}
	
	return func;
}

FieldList VarList(Node *n, FieldList varList){
	Node *child = n->child;
	
	// ParamDec
	FieldList tailList = ParamDec(child);
	varList->tail = tailList;
	child = child->sibling;
	
	// ParamDec COMMA VarList ?
	if(child != NULL){
		child = child->sibling;
		varList->tail = VarList(child, tailList);
	}
	
	return varList;
}

FieldList ParamDec(Node *n){
	Node *child = n->child;

	// Specifier VarDec
	Type type = Specifier(child);
	child = child->sibling;
	FieldList paramDec = VarDec(child, type);
	return paramDec;
}

/* Statements */
void CompSt(Node *n, Type retype){
	Node *child = n->child;
	
	// LC DefList StmtList RC
	child = child->sibling;
	DefList(child);
	child = child->sibling;
	StmtList(child, retype);
}

void StmtList(Node *n, Type retype){
	Node *child = n->child;

	// DefList -> NULL
	if(child == NULL)
			return;
	
	// Stmt StmtList
	Stmt(child, retype);
	StmtList(child->sibling, retype);
}

void Stmt(Node *n, Type retype){
	Node *child = n->child;

	// Exp SEMI
	if(strcmp(child->identifier, "Exp")==0){
		Exp(child);
	}
	// CompSt
	else if(strcmp(child->identifier, "CompSt")==0){
		CompSt(child);
	}
	// RETURN Exp SEMI
	else if(strcmp(child->identifier, "RETURN")==0){
		Type expType=Exp(child->sibling);
		if(expType==NULL||retype==NULL) return;
		if(!typeEqual(retype,expType)){
			//TODO: Error type 8
		}
	}
	// IF LP Exp Stmt (ELSE Stmt)?
	else if(strcmp(child->identifier, "IF")==0){i
		child = child->sibling->sibling;
		Exp(child);
		child = child->sibling->sibling;
		Stmt(child);
		child = child->sibling;
		if(child == NULL)
			return;
		child = child->sibling;
		Stmt(child);
	}
	// WHILE LP Exp RP Stmt
	else if(strcmp(child->identifier, "WHILE")==0){
		child = child->sibling->sibling;
		Exp(child);
		child = child->sibling->sibling;
		Stmt(child);
	}
	else
		exit(-1);
}

/* Local Definitions */
FieldList DefList(Node *n){
	Node *child = n->child;
	FieldList defList = NULL;
	
	// DefList -> NULL
	if(child==NULL)
			return defList;
	// Def DefList 
	defList = (FieldList)malloc(sizeof(struct FieldList_));
	defList->name = n->name;
	defList->type = NULL;
	Def(child, defList);
	FieldList tmp = defList;
	while(tmp->tail != NULL)
			tmp = tmp->tail;
	tmp>tail = DefList(child->sibling);
	return defList;
}

FieldList Def(Node *n, FieldList defList){
	Node *child = n->child;
	// Specifier DecList SEMI
	FieldList def = (FieldList)malloc(sizeof(struct FieldList_));
	def->type = Specifier(child);
	child = child->sibling;
	defList->tail = def;
	def->tail = DecList(child, def->type);
	return def;
}

FieldList DecList(Node *n, Type type){
	Node *child = n->child;
	FieldList decList = NULL;

	decList = Dec(child, type);
	child = child->sibling;

	// Dec
	if(child == NULL)
			return decList;
	// Dec COMMA DecList
	child = child->sibling;
	FieldList tmpDec = decList;
	while(tmpDec->tail != NULL)
			tmpDec = tmpDec->tail;
	tmpDec->tail = DecList(child, type);
	return decList;
}

FieldList Dec(Node *n, Type type){
	Node *child = n->child;
	FieldList dec = VarDec(child, type);

	child = child->sibling;
	// VarDec
	if(child == NULL)
			return dec;
	// VarDec ASSIGNOP Exp
	child = child->sibling;
	Type expType = Exp(child);
	if(!typeEqual(type, ExpType)){
		//TODO: Error Type 5
	}
	return dec;
}

/* Expressions */
//TODO: Complete Exp
Type Exp(Node *n){
	return NULL;
}
