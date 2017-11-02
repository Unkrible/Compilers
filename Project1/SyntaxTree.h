#ifndef SYNTAXTREE_H
#define SYNTAXTREE_H

#include <stdio.h>
#include <stdlib.h>

#define IDENTIFIER_LENGTH 32
#define VALUE_LENGTH 32

#define TYPE_KEYWORD 0
#define TYPE_KEYWORD_RELOP 1
#define TYPE_KEYWORD_BRACKET 2
#define TYPE_KEYWORD_PUNCTUATION 3
#define TYPE_KEYWORD_OPERATOR 4

#define TYPE_NONTERMINAL 65536

#define TYPE_ID 128

#define TYPE_INT 256
#define TYPE_OCT 257
#define TYPE_HEX 258

#define TYPE_FLOAT 512

#define TYPE_STRING 1024

extern int yylineno;

typedef int Type;

typedef struct Node{
	//TODO: Complete Node Structure

	char identifier[IDENTIFIER_LENGTH];
	char value[VALUE_LENGTH];
	Type type;
	int line;
	int height;

	// Children Brother Express
	struct Node *child;
	struct Node *sibling;

}Node;

extern Node* init(char identifier[], char value[], Type type, int line);
extern void insert(Node *p, Node *child);
extern void traverseTree(Node *p);
extern void cleanTree(Node *p);
#endif
