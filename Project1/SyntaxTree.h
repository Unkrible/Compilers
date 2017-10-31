#ifndef SYNTAXTREE_H
#define SYNTAXTREE_H

#include <stdio.h>
#include <stdlib.h>

#define IDENTIFIER_LENGTH 32
#define VALUE_LENGTH 32

#define TYPE_KEYWORD 0
#define TYPE_ID 1
#define TYPE_INT 2
#define TYPE_FLOAT 3
#define TYPE_STRING 4

extern int yylineno;

typedef int Type;

typedef struct Node{
	//TODO: Complete Node Structure

	char identifier[IDENTIFIER_LENGTH];
	Type type;
	int line;
	
	// Children Brother Express
	struct Node *child;
	struct Node *sibling;

}Node;

extern Node* init(char identifier[], Type type, int line);
extern void insert(Node *p, Node *child);
extern void traverseTree(Node *p);

#endif
