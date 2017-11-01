#include "SyntaxTree.h"
#include <string.h>

Node* init(char identifier[], char value[], Type type, int line){
	Node* root = malloc(sizeof(Node));
	
	strcpy(root->identifier, identifier);
	if(value != NULL)
		strcpy(root->value, value);

	root->type = type;
	root->line = line;

	return root;
}


void insert(Node *p, Node *child){
	if(p == NULL || child == NULL)
		return;

	if(p->child == NULL){
		p->child = child;
		return;
	}
	
	Node *temp = p->child;
	while(temp->sibling != NULL)
		temp = temp->sibling;
	temp->sibling = child;

}

void traverseTree(Node* p){
	if(p == NULL)
			return;

	//TODO: Print Node
	
	traverseTree(p->sibling);
	traverseTree(p->child);

}
