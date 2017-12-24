#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "SyntaxTree.h"
#include "Semantic.h"
#include <string.h>

#define HASH_SIZE 1024

typedef struct Entry_{
	char *name;
	Type type;
	struct Entry_ *next;
} Entry;

extern Entry *hashTable[HASH_SIZE];

void initTable();
Type getTable(char *name);

int varInsertTable(FieldList value);
int varInsertCheck(FieldList var);

int funcInsertTable(Function func);
int funcInsertCheck(Function func);

int structInsertTable(Structure structure);
int structInsertCheck(Structure structure);

void checkFuncDeclaration();
#endif
