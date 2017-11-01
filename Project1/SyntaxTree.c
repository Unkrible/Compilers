#include "SyntaxTree.h"
#include <string.h>

Node* init(char identifier[], char value[], Type type, int line){
	Node* root = malloc(sizeof(Node));
	
	strcpy(root->identifier, identifier);
	
	if(value != NULL)
		strcpy(root->value, value);
	else
		root->value[0]='\0';

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

	// Print Node
	switch(p->type){
		case TYPE_NONTERMINAL:
			// non-terminals	
			printf("%s (%d)\n",p->identifier,p->line);
			break;
		case TYPE_KEYWORD_BRACKET:
			// LP|RP|LB|RB|LC|RC
			printf("%s\n",p->identifier);
			break;
		default:
			// terminals
			printf("%s: %s\n",p->identifier,p->value);
			break;
	}

	traverseTree(p->child);
	traverseTree(p->sibling);

}
