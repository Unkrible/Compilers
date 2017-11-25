#include "Semantic.h"
#include "SyntaxTree.h"
#include "HashTable.h"
#include <string.h>

/* High-level Definitions */
void Program(Node* root){
	ExtDefList(root->child);
}

void ExtDefList(Node* n){
	if(n->child == NULL)
		return;
	ExtDef(n->child);
	ExtDefList(n->child->sibling);
}

/* Global Variable */
void ExtDef(Node *n){
	Node* child = n->child;
	Type type;
	type = Specifier(child);
	child = child->sibling;

	if(strcmp(child->identifier,"SEMI")==0)
			return;
	else if(strcmp(child->identifier,"ExtDecList")==0)
			ExtDecList(child,type);
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

void VarcDec(Node *n, Type type){
	child = n->child;
	if(child == NULL)
			return;

}
