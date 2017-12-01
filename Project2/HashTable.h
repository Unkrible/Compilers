#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "SyntaxTree.h"
#include "Semantic.h"
#include <string.h>

#define HASH_SIZE 32768

typedef struct Entry_{
	enum {FIELDLIST, FUNCTION, STRUCTURE} kind;
	union{
			FieldList value;
			Function func;
			Structure structure;
	}u;
	struct Entry_ *next;
}Entry;

extern Entry *hashTable[HASH_SIZE];

void initTable();

int insertTable(FieldList value);
int varInsertCheck(FieldList var);
FieldList getTable(char *name);

int insertTable(Function func);
int funcInsertCheck(Function func);

int insertTable(Structure structure);
int structInsertCheck(FieldList structure);
Structure getTable(char *name);

#endif
