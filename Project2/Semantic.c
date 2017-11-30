#include "Semantic.h"
#include "SyntaxTree.h"
#include "HashTable.h"
#include <string.h>

/* High-level Definitions */
void Program(Node* root){
	// ExtDefList
	ExtDefList(root->child);
}

void ExtDefList(Node* n){
	// ExtDefList -> NULL
	if(n->child == NULL)
		return;
	// ExtDef ExtDefList
	ExtDef(n->child);
	ExtDefList(n->child->sibling);
}

/* Global Variable */
void ExtDef(Node *n){
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
	// Specifier FunDec CompSt
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
			
			if(flag==ERROR_REDEFINE)
					printf("Error type 4 at line %d:
									Redefined function '%s'\n",
									func->line,func->name);
			else if(flag==ERROR_DECLARATION_CONFLICT)
					printf("Error type 19 at line %d:
									Inconsistent declaration of function
								   	'%s'\n",func->line,func->name);

			CompSt(child,type);
		}
	}
	else
			return;
}

void ExtDecList(Node *n, Type type){
	Node *child = n->child;
	if(child == NULL)
			return;
	VarDec(child, type);
	
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
	}
	// StructSpecifier
	else if(strcmp(child->identifier, "StructSpecifier")==0){
		type = StructSpecifier(child);
	}
	else
			exit(-1);

	return type;
}

Type StructSpecifier(Node *n){
	Node *child = n->child;
	if(child == NULL)
			exit(-1);

	Type type = (Type)malloc(sizeof(struct Type_));
	type->kind = STRUCTURE;
	FieldList fl = (FieldList)malloc(sizeof(struct FieldList_));
	fl->type = type;
	type->u.structure = fl;

	child = child->sibling;
	
	// STRUCT OptTag LC DefList RC
	if(strcmp(child->identifier, "OptTag")==0){
		// OptTag -> NULL
		if(child->child == NULL){
			fl->name = NULL;
		}
		// OptTag -> ID
		else{
			fl->name = child->child->value;
		}
		
		child = child->sibling->sibling;
		fl->tail = DefList(child);
		if(fl->name!=NULL){
			//TODO: Error type 16
			int flag = structInsertCheck(fl);
			insertTable(fl);
			if(flag == ERROR_REDEFINE)
					printf("Error type 16 at line %d: 
									Duplicated name '%s'\n'",
									n->child->line, fl->name);
		}
	}
	// STRUCT Tag
	else if(strcmp(child->identifier, "Tag")==0){
		//TODO: Error type 17
		FieldList result = getTable(child->child->value);
		if(result==NULL || result->type->kind!=STRUCTURE){
				printf("Error type 17 at line %d: 
									Undefined struct '%s'\n'",
									n->child->line, child->child->value);
		}
	}
	else
			exit(-1);

	return type;
}

/* Declarators */
FieldList VarDec(Node *n, Type type){
	Node *child = n->child;
	FieldList varDec=NULL;
	if(child == NULL)
			return;

	// ID
	if(strcmp(child->identifier, "ID")==0){
		//TODO: Error type 3 15
		varDec = (FieldList)malloc(sizeof(struct FieldList_));
		varDec->name = child->value;
		varDec->type = type;
		varDec->tail = NULL;
	}
	// VarDec LB INT RB
	else if(strcmp(child->identifier, "VarDec")==0){
		varDec = VarDec(child, type);
		if(varDec == NULL)	
				return NULL;

		child = child->sibling;
		child = child->sibling;
		Type tmpType = varDec->type;
		Type newType = (Type)malloc(sizeof(struct Type_));
		newType->kind = ARRAY;
		newType->u.array.size = (int)strtol(child->value,NULL,10);
		
		if(type->kind==BASIC || type->kind==STRUCTURE){
			newType->u.array.elem = type;
			varDec->type = newType;	
		}
		else if(type->kind == ARRAY){
			while((tmpType->u.array.elem)->kind == ARRAY)
					tmpType = tmpType->u.array.elem;
			newType->u.arrary.elem = tmpType;
			varDec->type = newType;
		}
	}
	else
		exit(-1);

	return varDec;
}

Function FunDec(Node *n, Type type){
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
		FieldList varList = (FieldList)malloc(sizeof(struct FieldList_));
		varList->name = func->name;
		varList->type = NULL;
		varList->tail = NULL:
		func->param = VarList(child, varList);
	}
	
	return func;
}

FieldList VarList(Node *n, FieldList varList){
	Node *child = n->child;
	
	// ParamDec
	FieldList tailList = ParamDec(child);
	varList->tail = tailList;
	child = child->sibling;
	
	// ParamDec COMMA VarList ?
	if(child != NULL){
		child = child->sibling;
		varList->tail = VarList(child, tailList);
	}
	
	return varList;
}

FieldList ParamDec(Node *n){
	Node *child = n->child;

	// Specifier VarDec
	Type type = Specifier(child);
	child = child->sibling;
	FieldList paramDec = VarDec(child, type);
	return paramDec;
}

/* Statements */
void CompSt(Node *n, Type retype){
	Node *child = n->child;
	
	// LC DefList StmtList RC
	child = child->sibling;
	DefList(child);
	child = child->sibling;
	StmtList(child, retype);
}

void StmtList(Node *n, Type retype){
	Node *child = n->child;

	// DefList -> NULL
	if(child == NULL)
			return;
	
	// Stmt StmtList
	Stmt(child, retype);
	StmtList(child->sibling, retype);
}

void Stmt(Node *n, Type retype){
	Node *child = n->child;

	// Exp SEMI
	if(strcmp(child->identifier, "Exp")==0){
		Exp(child);
	}
	// CompSt
	else if(strcmp(child->identifier, "CompSt")==0){
		CompSt(child);
	}
	// RETURN Exp SEMI
	else if(strcmp(child->identifier, "RETURN")==0){
		Type expType=Exp(child->sibling);
		if(expType==NULL||retype==NULL) return;
		if(!typeEqual(retype,expType)){
			//TODO: Error type 8
		}
	}
	// IF LP Exp Stmt (ELSE Stmt)?
	else if(strcmp(child->identifier, "IF")==0){i
		child = child->sibling->sibling;
		Exp(child);
		child = child->sibling->sibling;
		Stmt(child);
		child = child->sibling;
		if(child == NULL)
			return;
		child = child->sibling;
		Stmt(child);
	}
	// WHILE LP Exp RP Stmt
	else if(strcmp(child->identifier, "WHILE")==0){
		child = child->sibling->sibling;
		Exp(child);
		child = child->sibling->sibling;
		Stmt(child);
	}
	else
		exit(-1);
}

/* Local Definitions */
FieldList DefList(Node *n){
	Node *child = n->child;
	FieldList defList = NULL;
	
	// DefList -> NULL
	if(child==NULL)
			return defList;
	// Def DefList 
	defList = (FieldList)malloc(sizeof(struct FieldList_));
	defList->name = n->name;
	defList->type = NULL;
	Def(child, defList);
	FieldList tmp = defList;
	while(tmp->tail != NULL)
			tmp = tmp->tail;
	tmp>tail = DefList(child->sibling);
	return defList;
}

FieldList Def(Node *n, FieldList defList){
	Node *child = n->child;
	// Specifier DecList SEMI
	FieldList def = (FieldList)malloc(sizeof(struct FieldList_));
	def->type = Specifier(child);
	child = child->sibling;
	defList->tail = def;
	def->tail = DecList(child, def->type);
	return def;
}

FieldList DecList(Node *n, Type type){
	Node *child = n->child;
	FieldList decList = NULL;

	decList = Dec(child, type);
	child = child->sibling;

	// Dec
	if(child == NULL)
			return decList;
	// Dec COMMA DecList
	child = child->sibling;
	FieldList tmpDec = decList;
	while(tmpDec->tail != NULL)
			tmpDec = tmpDec->tail;
	tmpDec->tail = DecList(child, type);
	return decList;
}

FieldList Dec(Node *n, Type type){
	Node *child = n->child;
	FieldList dec = VarDec(child, type);

	child = child->sibling;
	// VarDec
	if(child == NULL)
			return dec;
	// VarDec ASSIGNOP Exp
	child = child->sibling;
	Type expType = Exp(child);
	if(!typeEqual(type, ExpType)){
		//TODO: Error Type 5
	}
	return dec;
}

/* Expressions */
Type Exp(Node *n)
{
	Node *child = n->child;
	if(strcmp(child->identifier,"LP")==0){
		//Exp->LP Exp RP
		child = child->sibling;
		return Exp(child);
	}
	else if(strcmp(child->identifier,"MINUS")==0){
		//Exp->MINUS Exp
		child = child->sibling;
		Type type2 = (Type)malloc(sizeof(struct Type_));
		type2 = Exp(child);
		if(type2==NULL)
			return NULL;
		if(type2->kind!=BASIC){
			printf("Error type 7 at line %d: Operands type mismatched\n", child->line);
			return NULL;
		}
		return type2;
	}
	else if(strcmp(child->identifier,"NOT")==0){
		//Exp->NOT Exp
		child = child->sibling;
		Type type2 = (Type)malloc(sizeof(struct Type_));
		type2 = Exp(child);
		if(type2==NULL)
			return NULL;
		if(type2->kind!=BASIC || type2->u.basic!=TYPE_INT){
			printf("Error type 7 at line %d: Operands type mismatched\n", child->line);
			return NULL;
		}
		return type2;
	}
	else if(strcmp(child->identifier,"ID")==0){
		if(child->sibling==NULL){
			//Exp->ID
			Type id = (Type)malloc(sizeof(struct Type_));
			id = isdefined(child->value);  //判断是否定义过
			if(id==NULL){
				printf("Error type 1 at line %d: Undefined variable \"%s\".\n", child->line, child->value);	
				return NULL;
			}
			return id;
		}
		else{
			//Exp->ID LP RP | ID LP Args RP
			Type type = (Type)malloc(sizeof(struct Type_));
			type = isdefined(child->value);   //判断是否定义过
			Function func = (Function)malloc(sizeof(struct Function_));            //如何定义
			if(type!=NULL&&func==NULL){
				printf("Error type 11 at line %d: \"%s\" is not a function.\n", child->line, child->value);
				return NULL;
			}
			if(func==NULL||func->isDefined==FALSE){
				printf("Error type 2 at line %d: Undefined function \"%s\".\n", child->line, child->value);
				return NULL;
			}
			Type param = (Type)malloc(sizeof(struct Type_));
			param = func->retype;
			child = child->sibling->sibling;
			if(strcmp(child->identifier,"RP")==0){
				if(param!=NULL){
					printf("Error type 9 at line %d: Function \"%s\" is not applicable for arguments.\n", child->line, func->name);
				}
			}
			else{
				if(cmptype(type,param)==FALSE){  //比较两个类型是否匹配
					printf("Error type 9 at line %d: Function \"%s\" is not applicable for arguments.\n", child->line, func->name);
				}
			}
			return func->retype;
		}
	}
	else if(strcmp(child->identifier,"INT")==0){
		//Exp->INT
		Type type = (Type)malloc(sizeof(struct Type_));
		type->kind = BASIC;
		type->detail.basic = TYPE_INT;
		return type;
	}
	else if(strcmp(child->identifier,"FLOAT")==0){
		//Exp->FLOAT
		Type type = (Type)malloc(sizeof(struct Type_));
		type->kind = BASIC;
		type->detail.basic = TYPE_FLOAT;
		return type;
	}
	else if(strcmp(child->identifier,"Exp")==0){
		//Exp->Exp ...
		if(strcmp(child->sibling->identifier,"ASSIGNOP")==0 ){
			//Exp->Exp ASSIGNOP Exp
			
			//判断表达式左边是否为左值
			Type Type1 = (Type)malloc(sizeof(struct Type_));
			if(isleftvalue(child)==true)   //判断是否为左值的函数
				Type1 = Exp(child);
			else{
				printf("Error type 6 at line %d: The left-hand side of an assignment must be a variable.\n", child->line);
				Type1 = NULL;
			}
			//判断赋值号两边表达式是否类型匹配
			Type Type2 = (Type)malloc(sizeof(struct Type_));
			Type2 = Exp(child->sibling->sibling);			
			if(ismatch(Type1, Type2)==TRUE)    //比较两个类型是否匹配的函数
				return Type1;
			else{
				if(Type1==NULL || Type2==NULL)
					//nothing
				else{
					printf("Error type 5 at line %d: Type mismatched for assignment.\n", child->line);
				}
				return NULL;			
			}
		}
		else if(strcmp(child->sibling->identifier,"AND")==0 || strcmp(child->sibling->identifier,"OR")==0 || strcmp(child->sibling->identifier,"RELOP")==0 || strcmp(child->sibling->identifier,"PLUS")==0 || strcmp(child->sibling->identifier,"MINUS")==0 || strcmp(child->sibling->identifier,"STAR")==0 || strcmp(child->sibling->identifier,"DIV")==0){
			//Exp->Exp AND|OR|RELOP|PLUS|MINUS|STAR|DIV Exp
			Type Type1 = (Type)malloc(sizeof(struct Type_));
			Type1 = Exp(child);
			Type Type2 = (Type)malloc(sizeof(struct Type_));
			Type2 = Exp(child->sibling->sibling);
			
			if(Type1->kind==BASIC && Type2->kind==BASIC && Type1->u.basic==Type2->u.basic)
				return Type1;
			else{
				if(Type1==NULL || Type2==NULL)
				//nothing
				else{
					printf("Error type 7 at line %d: Type mismatched for operands.\n", child->line);
				}
				return NULL;
			}
		}
		else if(strcmp(child->sibling->identifier,"LB")==0){
			//Exp->Exp LB Exp RB
			Type array =(Type)malloc(sizeof(struct Type_));
			array=Exp(child);
			
			if(array==NULL)
				return NULL;
			if(array->kind!=ARRAY){
				printf("Error type 10 at line %d: \"%s\" is not an array.\n", child->line, array->kind);
				return NULL;
			}
			child = child->sibling->sibling;
			Type arraynumber = (Type)malloc(sizeof(struct Type_));
			arraynumber = Exp(child);
			if(arraynumber==NULL)
				return NULL;
			if(arraynumber->kind!=BASIC || arraynumber->u.basic!=TYPE_INT){
				printf("Error type 12 at line %d: \"%s\" is not an array.\n", child->line , child->child->value);
				return NULL;
			}
			return array->u.array.elem;
		}
		else if(strcmp(child->sibling->identifier,"DOT")==0){
			//Exp->Exp DOT ID
			Type structure = (Type)malloc(sizeof(struct Type_));
			structure = Exp(child);
			if(structure==NULL)
				return NULL;
			if(structure->kind!=STRUCTURE){
				printf("Error type 13 at line %d: Illegal use of \".\".\n", child->line);
				return NULL;
			}
			Type structfield = (Type)malloc(sizeof(struct Type_));
			structfield = structure->u.structure->type;
			child = child->sibling->sibling;
			while(structfield!=NULL){
				if(strcmp( structfield->u.structure->name, child->value) ==0)
					return structfield;
				
				structfield = structfield->u.structure->tail->type;  //可能有错，找下一个
			}
			printf("Error type 14 at line %d: Non-existent field \"%s\".\n", child->line, child->value);
			return NULL;
		}

		return NULL;
	}

	return NULL;
}
