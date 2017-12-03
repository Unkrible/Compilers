#include "Semantic.h"
#include "SyntaxTree.h"
#include "HashTable.h"
#include <string.h>

int typeEqual(Type lhs, Type rhs){
	if(lhs==NULL && rhs==NULL)
		return 0;
	if(lhs==NULL || rhs==NULL)
		return 1;
	if(lhs->kind != rhs->kind)
		return 2;

	if(lhs->kind == BASIC){
		if(lhs->u.basic == rhs->u.basic)
				return 0;
		else
				return 3;
	}
	else if(lhs->kind == ARRAY){
		return typeEqual(lhs->u.array.elem, rhs->u.array.elem);
	}
	else if(lhs->kind == STRUCTURE){
		return structEqual(lhs->u.structure, rhs->u.structure);
	}
	else if(lhs->kind == FUNCTION){
		// It doesn't need to judge whether functions equal.
		exit(-2);
	}
	
	return 0;
}

int structEqual(Structure lhs, Structure rhs){
	int flag = 0;
	FieldList lDomain = lhs->domain;
	FieldList rDomain = rhs->domain;
	while(lDomain!=NULL && rDomain!=NULL){
		flag = typeEqual(lDomain->type, rDomain->type);
		if(flag != 0)
				return 4;
		lDomain = lDomain->tail;
		rDomain = rDomain->tail;
	}
	if(lDomain==NULL && rDomain==NULL)
			return 0;
	else
			return 4;
}

int valueEqual(FieldList lhs, FieldList rhs){
	if(lhs==NULL && rhs==NULL)
		return 0;
	if(lhs==NULL || rhs==NULL)
		return 1;
	if(valueEqual(lhs->tail,rhs->tail)!=0)
		return 2;
	return typeEqual(lhs->type, rhs->type);
}

/* High-level Definitions */
void Program(Node* root){
	//printf("%s\n",root->identifier);
	// ExtDefList

	ExtDefList(root->child);
	checkFuncDeclaration();
}

void ExtDefList(Node* n){
	if(n==NULL)
		return;
	//printf("%s\n",n->identifier);

	// ExtDefList -> NULL
	if(n->child == NULL)
		return;

	// ExtDef ExtDefList	
	ExtDef(n->child);
	ExtDefList(n->child->sibling);
}

/* Global Variable */
void ExtDef(Node *n){
	if(n==NULL)
		return;
	//printf("%s\n",n->identifier);
	
	Node* child = n->child;
	Type type;
	type = Specifier(child);
	child = child->sibling;

	// Specifier SEMI
	if(strcmp(child->identifier,"SEMI")==0)
			return;
	// Specifier ExtDecList SEMI
	else if(strcmp(child->identifier,"ExtDecList")==0)
			ExtDecList(child,type);
	// Specifier FunDec CompSt|SEMI
	else if(strcmp(child->identifier,"FunDec")==0){
		Function func=FunDec(child,type);
	
		child = child->sibling;
		int i;
		// Specifier FunDec CompSt
		if(strcmp(child->identifier,"CompSt")==0){
			if(func==NULL)
					return;
			func->isDefined = 1;
			int flag=funcInsertCheck(func);
			if(flag==ERROR_REDEFINE)
					printf("Error type 4 at Line %d: Redefined function \"%s\"\n",func->line,func->name);
			else if(flag==ERROR_DECLARATION_CONFLICT)
					printf("Error type 19 at Line %d: Inconsistent declaration of function \"%s\"\n",func->line,func->name);
			else if(flag!=FUNC_IS_DECLARED)
				funcInsertTable(func);
			CompSt(child,type);
		}
		// Specifier FunDec CompSt|SEMI
		else if(strcmp(child->identifier,"SEMI")==0){
			if(func==NULL)
					return;
			func->isDefined = 0;
			int flag=funcInsertCheck(func);
			if(flag==ERROR_REDEFINE)
					printf("Error type 4 at Line %d: Redefined function \"%s\"\n",func->line,func->name);
			else if(flag==ERROR_DECLARATION_CONFLICT)
					printf("Error type 19 at Line %d: Inconsistent declaration of function \"%s\"\n",func->line,func->name);
			else if(flag!=FUNC_IS_DECLARED)
				funcInsertTable(func);
		}
	}
	else
			return;
}

void ExtDecList(Node *n, Type type){
	if(n==NULL)
		return;
	//printf("%s\n",n->identifier);
	
	Node *child = n->child;
	if(child == NULL)
			return;
	VarDec(child, type, FROM_VARIABLE);
	
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
	if(n==NULL)
		return NULL;
	//printf("%s\n",n->identifier);
	
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
		return type;
	}
	// StructSpecifier
	else if(strcmp(child->identifier, "StructSpecifier")==0){
		return type = StructSpecifier(child);
	}
	else
			exit(-1);
}

Type StructSpecifier(Node *n){
	if(n==NULL)
		return NULL;
	//printf("%s\n",n->identifier);
	
	Node *child = n->child;
	if(child == NULL)
			exit(-1);

	child = child->sibling;
	
	// STRUCT OptTag LC DefList RC
	if(strcmp(child->identifier, "OptTag")==0){
		Type type = (Type)malloc(sizeof(struct Type_));
		type->kind = STRUCTURE;
		Structure structure = (Structure)malloc(sizeof(struct Structure_));
		type->u.structure = structure;

		// OptTag -> NULL
		if(child->child == NULL){
			structure->name = NULL;
		}
		// OptTag -> ID
		else{
			structure->name = child->child->value;
		}
		
		child = child->sibling->sibling;
		structure->domain = DefList(child,FROM_FIELD);
		if(structure->name!=NULL){
			int flag = structInsertCheck(structure);
			if(flag == ERROR_REDEFINE){
					printf("Error type 16 at Line %d: Duplicated name \"%s\".\n",n->child->line, structure->name);
					return NULL;
			}
			else{
					structInsertTable(structure);	
			}
		}
		return type;
	}
	// STRUCT Tag
	else if(strcmp(child->identifier, "Tag")==0){
		Type result = getTable(child->child->value);
		if(result==NULL || result->kind!=STRUCTURE || 
			strcmp(result->u.structure->name,child->child->value)!=0){
				printf("Error type 17 at Line %d: Undefined structure \"%s\".\n",n->child->line, child->child->value);
		}
		return result;
	}
}

/* Declarators */
FieldList VarDec(Node *n, Type type, int from){
	if(n==NULL)
		return NULL;
	//printf("%s\n",n->identifier);
	
	Node *child = n->child;
	FieldList varDec=NULL;
	if(child == NULL)
			return NULL;

	// ID
	if(strcmp(child->identifier, "ID")==0){
		varDec = (FieldList)malloc(sizeof(struct FieldList_));
		varDec->name = child->value;
		varDec->type = type;
		varDec->tail = NULL;
		if(from==FROM_PARAM)
				return varDec;
		int flag = varInsertCheck(varDec);
		if(flag==ERROR_REDEFINE){
				if(from==FROM_VARIABLE){
					printf("Error type 3 at Line %d: Redefined variable \"%s\".\n",child->line, varDec->name);
				}
				else{
					printf("Error type 15 at Line %d: Redefined field \"%s\".\n",child->line, varDec->name);
				}
				return NULL;
		}
		else
				varInsertTable(varDec);
		return varDec;
	}
	// VarDec LB INT RB
	else if(strcmp(child->identifier, "VarDec")==0){

		Type varDec = (Type)malloc(sizeof(struct Type_));
		varDec->kind = ARRAY;
		varDec->u.array.size = (int)strtol(child->sibling->sibling->value,NULL,10);
		varDec->u.array.elem = type; 

		return VarDec(child, varDec, from);
	}
}

Function FunDec(Node *n, Type type){
	if(n==NULL)
		return NULL;
	//printf("%s\n",n->identifier);
	
	Node *child = n->child;
	Function func = (Function)malloc(sizeof(struct Function_));
	func->name = child->value;
	func->isDefined = 0;
	func->param = NULL;
	func->retype = type;
	func->line = child->line;

	child = child->sibling->sibling;
	// ID LP VarList RP
	if(strcmp(child->identifier,"VarList")==0){
		func->param = VarList(child);
	}
	return func;
}

FieldList VarList(Node *n){
	if(n==NULL)
			return NULL;
	//printf("%s\n",n->identifier);
	
	Node *child = n->child;
	// ParamDec
	FieldList varList = ParamDec(child);
	child = child->sibling;
	
	// ParamDec COMMA VarList ?
	if(child != NULL){
		child = child->sibling;
		varList->tail = VarList(child);
	}
	
	return varList;
}

FieldList ParamDec(Node *n){
	if(n==NULL)
		return NULL;
	//printf("%s\n",n->identifier);
	
	Node *child = n->child;
	// Specifier VarDec
	Type type = Specifier(child);
	child = child->sibling;
	FieldList paramDec = VarDec(child, type, FROM_PARAM);
	return paramDec;
}

/* Statements */
void CompSt(Node *n, Type retype){
	if(n==NULL)
		return;
	//printf("%s\n",n->identifier);
	
	Node *child = n->child;
	// LC DefList StmtList RC
	child = child->sibling;
	DefList(child, FROM_VARIABLE);
	child = child->sibling;
	StmtList(child, retype);
}

void StmtList(Node *n, Type retype){
	if(n==NULL)
		return;
	//printf("%s\n",n->identifier);

	Node *child = n->child;
	// StmtList -> NULL
	if(child == NULL)
			return;
	
	// Stmt StmtList
	Stmt(child, retype);
	StmtList(child->sibling, retype);
}

void Stmt(Node *n, Type retype){
	if(n==NULL)
		return;
	//printf("%s\n",n->identifier);
	
	Node *child = n->child;
	// Exp SEMI
	if(strcmp(child->identifier, "Exp")==0){
		Exp(child);
	}
	// CompSt
	else if(strcmp(child->identifier, "CompSt")==0){
		CompSt(child, retype);
	}
	// RETURN Exp SEMI
	else if(strcmp(child->identifier, "RETURN")==0){
		Type expType=Exp(child->sibling);
		if(expType==NULL||retype==NULL) return;
		if(typeEqual(retype,expType)!=0){
			printf("Error type 8 at Line %d: Type mismatched for return.\n",child->line);
		}
	}
	// IF LP Exp Stmt (ELSE Stmt)?
	else if(strcmp(child->identifier, "IF")==0){
		child = child->sibling->sibling;
		Type expType=Exp(child);
		child = child->sibling->sibling;
		Stmt(child, retype);
		child = child->sibling;
		if(child == NULL)
			return;
		child = child->sibling;
		Stmt(child, retype);
	}
	// WHILE LP Exp RP Stmt
	else if(strcmp(child->identifier, "WHILE")==0){
		child = child->sibling->sibling;
		Exp(child);
		child = child->sibling->sibling;
		Stmt(child, retype);
	}
}

/* Local Definitions */
FieldList DefList(Node *n, int from){
	if(n==NULL)
		return NULL;
	//printf("%s\n",n->identifier);
	
	Node *child = n->child;
	FieldList defList = NULL;
	
	// DefList -> NULL
	if(child==NULL)
			return defList;
	// Def DefList 
	defList = Def(child, from);

	if(defList == NULL){
		defList = DefList(child->sibling, from);
		return defList;
	}

	FieldList tmp = defList;
	while(tmp->tail != NULL)
			tmp = tmp->tail;
	tmp->tail = DefList(child->sibling, from);
	return defList;
}

FieldList Def(Node *n, int from){
	if(n==NULL)
		return NULL;
	//printf("%s\n",n->identifier);
	
	Node *child = n->child;
	// Specifier DecList SEMI
	FieldList def = NULL;
	Type type = Specifier(child);
	child = child->sibling;
	def = DecList(child, type, from);
	return def;
}

FieldList DecList(Node *n, Type type, int from){
	if(n==NULL)
		return NULL;
	//printf("%s\n",n->identifier);
	
	Node *child = n->child;
	FieldList decList = NULL;

	decList = Dec(child, type, from);
	child = child->sibling;

	// Dec
	if(child == NULL)
			return decList;
	// Dec COMMA DecList
	child = child->sibling;
	if(decList==NULL){
		decList = DecList(child, type, from);
	}
	else{
		FieldList tmpDec = decList;
		while(tmpDec->tail != NULL)
				tmpDec = tmpDec->tail;
		tmpDec->tail = DecList(child, type, from);
	}
	return decList;
}

FieldList Dec(Node *n, Type type, int from){
	if(n==NULL)
		return NULL;
	//printf("%s\n",n->identifier);
	
	Node *child = n->child;
	FieldList dec = VarDec(child, type, from);

	child = child->sibling;
	// VarDec
	if(child == NULL)
			return dec;
	// VarDec ASSIGNOP Exp
	if(from == FROM_FIELD){
		printf("Error type 15 at Line %d: be initialized field \"%s\"\n",child->line, dec->name);
		return NULL;
	}
	child = child->sibling;
	Type expType = Exp(child);
	if(typeEqual(type, expType)!=0){
		printf("Error type 5 at Line %d: The type mismatched\n",child->line);
		return NULL;
	}
	return dec;
}

/* Expressions */
Type Exp(Node *n){
	if(n==NULL)
		return NULL;
	//printf("%s\n",n->identifier);
	
	Node *child = n->child;
	//printf("%s\n",child->identifier);
	if(strcmp(child->identifier,"LP")==0){
		//Exp->LP Exp RP
		child = child->sibling;
		return Exp(child);
	}
	else if(strcmp(child->identifier,"MINUS")==0){
		//Exp->MINUS Exp
		child = child->sibling;
		Type type = NULL;
		type = Exp(child);
		if(type==NULL)
			return NULL;
		if(type->kind!=BASIC){
			printf("Error type 7 at Line %d: Operands type mismatched\n", child->line);
			return NULL;
		}
		Type rtn = (Type)malloc(sizeof(struct Type_));
		memcpy(rtn, type, sizeof(struct Type_));
		rtn->assign = RIGHT;
		return rtn;
	}
	else if(strcmp(child->identifier,"NOT")==0){
		//Exp->NOT Exp
		child = child->sibling;
		Type type = NULL;
		type = Exp(child);
		if(type==NULL)
			return NULL;
		if(type->kind!=BASIC || type->u.basic!=TYPE_INT){
			printf("Error type 7 at Line %d: Operands type mismatched\n", child->line);
			return NULL;
		}
		Type rtn = (Type)malloc(sizeof(struct Type_));
		memcpy(rtn, type, sizeof(struct Type_));
		rtn->assign = RIGHT;
		return rtn;
	}
	else if(strcmp(child->identifier,"ID")==0){
		if(child->sibling==NULL){
			//Exp->ID
			Type value = getTable(child->value);  //判断是否定义过
			if(value==NULL||value->kind==FUNCTION){
				printf("Error type 1 at Line %d: Undefined variable \"%s\".\n", child->line, child->value);	
				return NULL;
			}
			Type rtn = (Type)malloc(sizeof(struct Type_));
			memcpy(rtn, value, sizeof(struct Type_));
			rtn->assign = BOTH;
			return rtn;
		}
		else{
			//Exp->ID LP RP | ID LP Args RP
			Type func = getTable(child->value);   //判断是否定义过
			if(func==NULL){
				printf("Error type 2 at Line %d: Undefined function \"%s\".\n", child->line, child->value);
				return NULL;
			}
			if(func->kind!=FUNCTION){
				printf("Error type 11 at Line %d: \"%s\" is not a function.\n", child->line, child->value);
				return NULL;
			}
			if(func->u.function->isDefined==0){
				printf("Error type 2 at Line %d: Undefined function \"%s\".\n", child->line, child->value);
				return NULL;
			}
			FieldList param = func->u.function->param;
			child = child->sibling->sibling;
			if(strcmp(child->identifier,"RP")==0){
				if(param!=NULL){
					printf("Error type 9 at Line %d: Function \"%s\" is not applicable for arguments.\n", child->line, func->u.function->name);
					return NULL;
				}
			}
			else{
				if(Args(child,param)!=0){  //比较两个类型是否匹配
					printf("Error type 9 at Line %d: Function \"%s\" is not applicable for arguments.\n", child->line, func->u.function->name);
					return NULL;
				}
			}
			Type rtn = (Type)malloc(sizeof(struct Type_));
			memcpy(rtn, func->u.function->retype, sizeof(struct Type_));
			rtn->assign = RIGHT;
			return rtn;
		}
	}
	else if(strcmp(child->identifier,"INT")==0){
		//Exp->INT
		Type type = (Type)malloc(sizeof(struct Type_));
		type->kind = BASIC;
		type->u.basic = TYPE_INT;
		type->assign = RIGHT;
		return type;
	}
	else if(strcmp(child->identifier,"FLOAT")==0){
		//Exp->FLOAT
		Type type = (Type)malloc(sizeof(struct Type_));
		type->kind = BASIC;
		type->u.basic = TYPE_FLOAT;
		type->assign = RIGHT;
		return type;
	}
	else if(strcmp(child->identifier,"Exp")==0){
		//Exp->Exp ...
		if(strcmp(child->sibling->identifier,"ASSIGNOP")==0 ){
			//Exp->Exp ASSIGNOP Exp
			
			//判断表达式左边是否为左值
			Type lhs = Exp(child);
			Type rhs = Exp(child->sibling->sibling);
			if(lhs==NULL || rhs==NULL)
				return NULL;
			if(lhs->assign==RIGHT){
				printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n", child->line);
				return NULL;
			}
			//判断赋值号两边表达式是否类型匹配
			if(typeEqual(lhs, rhs)==0){    
				return lhs;
			}
			else{
				printf("Error type 5 at Line %d: Type mismatched for assignment.\n", child->line);
				return NULL;			
			}
		}
		else if(strcmp(child->sibling->identifier,"AND")==0 || 
						strcmp(child->sibling->identifier,"OR")==0 || 
						strcmp(child->sibling->identifier,"RELOP")==0 || 
						strcmp(child->sibling->identifier,"PLUS")==0 || 
						strcmp(child->sibling->identifier,"MINUS")==0 || 
						strcmp(child->sibling->identifier,"STAR")==0 || 
						strcmp(child->sibling->identifier,"DIV")==0){
			//Exp->Exp AND|OR|RELOP|PLUS|MINUS|STAR|DIV Exp
			Type lhs = Exp(child);
			Type rhs = Exp(child->sibling->sibling);
		
			if(lhs==NULL||rhs==NULL)
				return NULL;	
			if(lhs->kind==BASIC && rhs->kind==BASIC && 
							lhs->u.basic==rhs->u.basic){
				Type rtn = (Type)malloc(sizeof(struct Type_));
				memcpy(rtn, lhs, sizeof(struct Type_));
				rtn->assign = RIGHT;
				return rtn;
			}
			else{
				printf("Error type 7 at Line %d: Type mismatched for operands.\n", child->line);
				return NULL;
			}
		}
		else if(strcmp(child->sibling->identifier,"LB")==0){
			//Exp->Exp LB Exp RB
			Type array=Exp(child);
			
			if(array==NULL)
				return NULL;
			if(array->kind!=ARRAY){
				printf("Error type 10 at Line %d: \"%s\" is not an array.\n", child->line, child->child->value);
				return NULL;
			}
			child = child->sibling->sibling;
			Type arrayNumber = Exp(child);
			if(arrayNumber==NULL)
				return NULL;
			if(arrayNumber->kind!=BASIC || arrayNumber->u.basic!=TYPE_INT){
				printf("Error type 12 at Line %d: \"%s\" is not an integer.\n", child->line , child->child->value);
				return NULL;
			}
			Type rtn = (Type)malloc(sizeof(struct Type_));
			memcpy(rtn, array->u.array.elem, sizeof(struct Type_));
			rtn->assign = BOTH;
			return rtn;
		}
		else if(strcmp(child->sibling->identifier,"DOT")==0){
			//Exp->Exp DOT ID
			Type structure = Exp(child);
			if(structure==NULL)
				return NULL;
			if(structure->kind!=STRUCTURE){
				printf("Error type 13 at Line %d: Illegal use of \".\".\n", child->line);
				return NULL;
			}
			FieldList structDomain = structure->u.structure->domain;
			child = child->sibling->sibling;
			while(structDomain!=NULL){
				if(strcmp(structDomain->name, child->value) ==0){
					Type rtn = (Type)malloc(sizeof(struct Type_));
					memcpy(rtn, structDomain->type, sizeof(struct Type_));
					rtn->assign = BOTH;
					return rtn;
				}
				structDomain = structDomain->tail;  //可能有错，找下一个
			}
			printf("Error type 14 at Line %d: Non-existent field \"%s\".\n", child->line, child->value);
			return NULL;
		}

		return NULL;
	}

	return NULL;
}

int Args(Node *n, FieldList param){
	if(n==NULL)
		return 1;
	//printf("%s\n",n->identifier);
	
	Node *child = n->child;
	if(param==NULL)
			return 1;

	Type tmpParam = Exp(child);
	if(typeEqual(param->type, tmpParam)==0){
		if(child->sibling==NULL)
				return 0;
		else
				return Args(child->sibling->sibling, param->tail);
	}
	else
		return 2;
}
