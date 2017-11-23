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
