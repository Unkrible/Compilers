#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "SyntaxTree.h"
#include "Semantic.h"
#include <string.h>

#define HASH_SIZE 32768

typedef struct Entry_{
	enum {FIELDLIST,FUNCTION} kind;
	union{
			FieldList value;
			Function func;
	}u;
	struct Entry_ *next;
}Entry;

extern Entry *hashTable[HASH_SIZE];

void initTable();
int insertTable(FieldList value);
int insertTable(Function func);
int funcInsertCheck(Function func);
FieldList getTable(char *name);

#endif
