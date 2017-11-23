#include "HashTable.h"

Entry *hashTable[HASH_SIZE];

unsigned int hashPJW(char *name){
	unsigned int val=0, i;
	for(; *name; ++name){
		val = (val<<2)+*name;
		if(i= val& ~0x3fff)
				val = (val^(i>>12))&0x3fff;
	}
	return val;
}

void initTable(){
	int i=0;
	for(;i<HASH_SIZE;i++){
		hashTable[i]=NULL;
	}
}

int insertTable(FieldList value){

	if(hashTable==NULL || value==NULL)
			return 0;

	unsigned int hashValue = 0;
	hashValue = hashPJW(value->name)%HASH_SIZE;
	
	Entry *curEntry = (Entry *)malloc(sizeof(Entry));
	curEntry->next = hashTable[hashValue];
	hashTable[hashValue] = curEntry;
	
	curEntry->kind = FIELDLIST;
	curEntry->u.value = value;

	return 1;
}

int insertTable(Function func){
	if(hashTalbe==NULL || func==NULL)
			return 0;

	unsigned int hashValue = 0;
	hashValue = hashPJW(value->name)%HASH_SIZE;

	Entry *curEntry = (Entry *)malloc(sizeof(Entry));
	curEntry->next = hashTable[hashValue];
	hashTable[hashValue] = curEntry;

	curEntry->kind = FUNCTION;
	curEntry->u.func = func;

	return 1;
}

int funcInsertCheck(Function func){

	if(hashTable==NULL || func==NULL)
			return 0;
	
	unsigned int hashValue = hashPJW(func->name)%HASH_SIZE;
	Entry *tmp = hashTable[hashValue];
	for(;tmp!=NULL;tmp=tmp->next){
		if(tmp->kind != FUNCTION)
				continue;
		Function tmpFunc = tmp->u.func;
		if(strcmp(tmpFunc->name,func->name)!=0)
				continue;
		if(paramCmp(tmpFunc->param,func->param)==0 
						&& retypeCmp(tmpFunc->retype,func->retype==0))
				continue;
		if(tmpFunc->isDefined==0){
			tmpFunc->isDefined = func->isDefined;
			continue;
		}
		return 0;	
	}
	return 1;
}

FieldList getTable(char *name){
	
	if(hashTable==NULL || name==NULL)
			return NULL;

	unsigned int hashValue = hashPJW(name)%HASH_SIZE;

	Entry *tmp = hashTable[hashValue];
	for(;tmp!=NULL;tmp=tmp->next){
		if(strcmp(tmp->value->name,name)==0)
				return tmp->value;
	}

	// Error: No such name
	return NULL;
}
