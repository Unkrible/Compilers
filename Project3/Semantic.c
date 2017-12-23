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
	int flag=0;
	FieldList lstructDomain = lhs->domain;
	FieldList rstructDomain = rhs->domain;
	while(lstructDomain!=NULL&&rstructDomain!=NULL){
		flag=typeEqual(lstructDomain->type,rstructDomain->type);
		if(flag!=0){
			return 1;
		}
		lstructDomain = lstructDomain->tail;
		rstructDomain = rstructDomain->tail;
	}
	if(lstructDomain==NULL&&rstructDomain==NULL)
		return 0;
	else
		return 1;
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

int typeSize(Type type){
	if(type->kind==BASIC||type->kind==FUNCTION)
	{
		if(type->u.basic==TYPE_INT)
			return 4;
		else return 8;
	}
	else if(type->kind==STRUCTURE)	//struct
	{
		int size=0;
		FieldList f=type->u.structure->domain;
		while(f!=NULL)
		{
			size+=typeSize(f->type);
			f=f->tail;
		}
		return size;
	}
	else if(type->kind==ARRAY)		//array
	{
		//高维数组
		if(type->u.array.elem->kind==ARRAY)
		{
			printf("Can not translate the code: Contain multidimensional array and function parameters of array type!\n");
			exit(-1);
		}
		return	type->u.array.size*typeSize(type->u.array.elem);
	}
	printf("type size error!\n");
	return 0;
}

/* High-level Definitions */
void Program(Node* root){
	printf("%s\n",root->identifier);
	// ExtDefList

	ExtDefList(root->child);
	checkFuncDeclaration();
}

void ExtDefList(Node* n){
	if(n==NULL)
		return;
	printf("%s\n",n->identifier);

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
	printf("%s\n",n->identifier);

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
			else if(flag!=FUNC_IS_DECLARED){
				funcInsertTable(func);
				Operand funcop=malloc(sizeof(Operand_));
				funcop->kind = FUNCTION;
				funcop->u.value = func->name;
				InterCode code=malloc(sizeof(InterCode_));
				code->kind = FUNCTION_N;
				code->u.sinop.op = funcop;
				insertCode(code);		//funtion  :
				FieldList param=func->param;
				while(param!=NULL)
				{
					Operand pop=malloc(sizeof(Operand_));
					pop->kind=VARIABLE;
					pop->u.value = param->name;
					InterCode pcode = malloc(sizeof(InterCode_));
					pcode->kind = PARAM_N;
					pcode->u.sinop.op=pop;
					insertCode(pcode);
					param=param->tail;
				}
			}
			CompSt(child,type);
		}
		// Specifier FunDec SEMI
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
	printf("%s\n",n->identifier);

	Node *child = n->child;
	if(child == NULL)
			return;
	FieldList fl = VarDec(child, type, FROM_VARIABLE);
	if(fl != NULL)
	{
		if(fl->type->kind == ARRAY)			//array
		{
			Operand op=malloc(sizeof(Operand_));
			op->kind = TEMPVAR;
			op->u.var_no = temVarNo++;

			InterCode deccode=malloc(sizeof(InterCode_));
			deccode->kind=DEC_N;
			deccode->u.dec.op = op;
			deccode->u.dec.size=typeSize(fl->type);
			insertCode(deccode);

			Operand v=malloc(sizeof(Operand_));
			v->kind = VARIABLE;
			v->u.value = fl->name;

			InterCode addrcode = malloc(sizeof(InterCode_));
			addrcode->kind = RIGHTAT_N;
			addrcode->u.assign.left = v;
			addrcode->u.assign.right = op;
			insertCode(addrcode);
		}
	}
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
	printf("%s\n",n->identifier);

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
	printf("%s\n",n->identifier);

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
	printf("%s\n",n->identifier);

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
		else{
			if(type->kind==STRUCTURE && from==FROM_FIELD)
				Operand op=malloc(sizeof(Operand_));
				op->kind = TEMPVAR;
				op->u.var_no = temVarNo++;

				InterCode deccode=malloc(sizeof(InterCode_));
				deccode->kind = DEC_N;
				deccode->u.dec.op = op;
				deccode->u.dec.size = typeSize(type);
				insertCode(deccode);

				Operand v=malloc(sizeof(Operand_));
				v->kind = VARIABLE;
				v->u.value = child->value;

				InterCode addrcode=malloc(sizeof(InterCode_));
				addrcode->kind = RIGHTAT_N;
				addrcode->u.assign.left = v;
				addrcode->u.assign.right = op;
				insertCode(addrcode);
				varInsertTable(varDec);
		}
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
	printf("%s\n",n->identifier);

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
	printf("%s\n",n->identifier);

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
	printf("%s\n",n->identifier);

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
	printf("%s\n",n->identifier);

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
	printf("%s\n",n->identifier);

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
	printf("%s\n",n->identifier);

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
		Operand op=malloc(sizeof(Operand_));
		op->kind = TEMPVAR;
		op->u.var_no= temVarNo++;
		Type expType=Exp(child->sibling);
		if(expType==NULL||retype==NULL) return;
		if(typeEqual(retype,expType)!=0){
			printf("Error type 8 at Line %d: Type mismatched for return.\n",child->line);
		}
		InterCode code = malloc(sizeof(InterCode_));
		code->kind = RETURN_N;
		code->u.sinop.op = op;
		insertCode(code);
	}
	// IF LP Exp Stmt (ELSE Stmt)?
	else if(strcmp(child->identifier, "IF")==0){
		child = child->sibling->sibling;
		Operand lb1 = malloc(sizeof(Operand_));
		lb1->kind = LABEL;
		lb1->u.var_no = labelNo++;
		Operand lb2 = malloc(sizeof(Operand_));
		lb2->kind= LABEL;
		lb2->u.var_no = labelNo++;
		Type expType = Exp_Cond(child,lb1,lb2);

		InterCode code1=malloc(sizeof(InterCode_));
		code1->kind=LABEL_N;
		code1->u.sinop.op=lb1;
		insertCode(code1);

		child = child->sibling->sibling;
		Stmt(child, retype);
		InterCode lb2code = malloc(sizeof(InterCode_));
		lb2code->kind = LABEL_N;
		lb2code->u.sinop.op = lb2;
		child = child->sibling;
		if(child == NULL){
			insertCode(lb2code);
			return;
		}
		Operand lb3 = malloc(sizeof(Operand_));
		lb3->kind = LABEL;
		lb3->u.var_no = labelNo++;
		InterCode code2=malloc(sizeof(InterCode_));
		code2->kind = GOTO_N;
		code2->u.sinop.op = lb3;
		insertCode(code2);			//goto label3
		insertCode(lb2code);		//label2
		child = child->sibling;
		Stmt(child, retype);
		InterCode lb3code=malloc(sizeof(InterCode_));
		lb3code->kind = LABEL_N;
		lb3code->u.sinop.op = lb3;
		insertCode(lb3code);		//label3
	}
	// WHILE LP Exp RP Stmt
	else if(strcmp(child->identifier, "WHILE")==0){
		Operand lb1=malloc(sizeof(Operand_));
		lb1->kind = LABEL;
		lb1->u.var_no = labelNo++;
		Operand lb2=malloc(sizeof(Operand_));
		lb2->kind = LABEL;
		lb2->u.var_no = labelNo++;
		Operand lb3 = malloc(sizeof(Operand_));
		lb3->kind = LABEL;
		lb3->u.var_no = labelNo++;
		child = child->sibling->sibling;

		InterCode lb1code = malloc(sizeof(InterCode_));
		lb1code->kind = LABEL_N;
		lb1code->u.sinop.op = lb1;
		insertCode(lb1code);		//label 1
		Exp_Cond(child,lb2,lb3);	//code1

		InterCode lb2code=malloc(sizeof(InterCode_));
		lb2code->kind = LABEL_N;
		lb2code->u.sinop.op = lb2;
		insertCode(lb2code);		//label 2
		child = child->sibling->sibling;
		Stmt(child, retype);
		InterCode gotolb1 = malloc(sizeof(InterCode_));
		gotolb1->kind = GOTO_N;
		gotolb1->u.sinop.op = lb1;
		insertCode(gotolb1);		//goto label1
		InterCode lb3code = malloc(sizeof(InterCode_));
		lb3code->kind = LABEL_N;
		lb3code->u.sinop.op = lb3;
		insertCode(lb3code);		//label3
	}
}

/* Local Definitions */
FieldList DefList(Node *n, int from){
	if(n==NULL)
		return NULL;
	printf("%s\n",n->identifier);

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
	printf("%s\n",n->identifier);

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
	printf("%s\n",n->identifier);

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
	printf("%s\n",n->identifier);

	Node *child = n->child;
	FieldList dec = VarDec(child, type, from);

	if(f->type->kind==1&&from==1)
	{
		//array space
		Operand op=malloc(sizeof(Operand_));
		op->kind=TEMPVAR;
		op->u.var_no=temVarNo++;

		InterCode deccode=malloc(sizeof(InterCode_));
		deccode->kind=DEC_N;
		deccode->u.dec.op=op;
		deccode->u.dec.size=typeSize(dec->type);
		insertCode(deccode);

		Operand v=malloc(sizeof(Operand_));
		v->kind=VARIABLE;
		v->u.value=dec->name;

		InterCode addrcode=malloc(sizeof(InterCode_));
		addrcode->kind = RIGHTAT_N;
		addrcode->u.assign.left = v;
		addrcode->u.assign.right = op;
		insertCode(addrcode);
	}

	child = child->sibling;
	// VarDec
	if(child == NULL)
			return dec;
	// VarDec ASSIGNOP Exp
	if(from == FROM_FIELD){
		printf("Error type 15 at Line %d: be initialized field \"%s\"\n",child->line, dec->name);
		return NULL;
	}
	Operand place=malloc(sizeof(Operand_));
	place->kind=VARIABLE;
	place->u.value=dec->name;
	child = child->sibling;
	Type expType = Exp(child, place);
	if(typeEqual(type, expType)!=0){
		printf("Error type 5 at Line %d: The type mismatched\n",child->line);
		return NULL;
	}
	if(place->kind!=VARIABLE||place->u.value!=dec->name)
	{
		Operand left=malloc(sizeof(Operand_));
		left->kind=VARIABLE;
		left->u.value=dec->name;
		InterCode asscode=malloc(sizeof(InterCode_));
		asscode->kind=ASSIGN_N;
		asscode->u.assign.left=left;
		asscode->u.assign.right=place;
		insertCode(asscode);
	}
	return dec;
}

/* Expressions */
Type Exp(Node *n, Operand place){
	if(n==NULL)
		return NULL;
	printf("%s\n",n->identifier);

	Node *child = n->child;
	printf("%s\n",child->identifier);
	if(strcmp(child->identifier,"LP")==0){
		//Exp->LP Exp RP
		child = child->sibling;
		return Exp(child,place);
	}
	else if(strcmp(child->identifier,"MINUS")==0){

		//--------------------------translate----------------
			Operand rightOp = malloc(sizeof(Operand_));
			memset(rightOp, 0, sizeof(Operand_));
			rightOp->kind = TEMPVAR;
			rightOp->u.var_no = temVarNo;
			int rightOpNo = rightOp->u.var_no;
			temVarNo++;
		//--------------------------translate-----------------

		//Exp->MINUS Exp
		child = child->sibling;
		Type type = NULL;
		type = Exp(child,rightOp);
		if(type==NULL)
			return NULL;
		if(type->kind!=BASIC){
			printf("Error type 7 at Line %d: Operands type mismatched\n", child->line);
			return NULL;
		}

		//--------------------------translate-----------------
			Operand zeroOp =  malloc(sizeof(Operand_));
			memset(zeroOp, 0, sizeof(Operand_));
			zeroOp->kind = CONST_OP;
			zeroOp->u.value = zeroStr;
			// place := #0 - Exp
			if(place!=NULL){
				InterCode minusCode = malloc(sizeof(InterCode_));
				memset(minusCode, 0, sizeof(InterCode_));
				minusCode->kind = SUB_N;
				minusCode->u.binop.result = place;
				minusCode->u.binop.op1 = zeroOp;
				minusCode->u.binop.op2 = rightOp;
				insertCode(minusCode);
			}
		//--------------------------translate-----------------

		type->assign = RIGHT;
		return type;
	}
	else if(strcmp(child->identifier,"NOT")==0){
		//Exp->NOT Exp

		Operand lb1=malloc(sizeof(Operand_));
		lb1->kind=LABEL;
		lb1->u.var_no=labelNo++;
		Operand lb2=malloc(sizeof(Operand_));
		lb2->kind=LABEL;
		lb2->u.var_no=labelNo++;

		InterCode code0=malloc(sizeof(InterCode_));
		code0->kind=ASSIGN_N;
		code0->u.assign.left=place;
		Operand c0=malloc(sizeof(Operand_));
		c0->kind=CONSTANT;
		c0->u.value= zeroStr;
		code0->u.assign.right=c0;
		if(place!=NULL)
			insertCode(code0);	//code0
		Type t=Exp_Cond(n,lb1,lb2);	//code1

		InterCode lb1code=malloc(sizeof(InterCode_));
		lb1code->kind=LABEL_K;
		lb1code->u.sinop.op=lb1;
		insertCode(lb1code);	//label 1

		Operand c1=malloc(sizeof(Operand_));
		c1->kind=CONSTANT;
		c1->u.value=oneStr;
		InterCode code2=malloc(sizeof(InterCode_));
		code2->kind=ASSIGN_N;
		code2->u.assign.left=place;
		code2->u.assign.right=c1;
		if(place!=NULL)
			insertCode(code2);		//code2
		InterCode lb2code=malloc(sizeof(InterCode_));
		lb2code->kind=LABEL_N;
		lb2code->u.sinop.op=lb2;
		insertCode(lb2code);
		t->assign = RIGHT
		return t;
	}
	else if(strcmp(child->identifier,"ID")==0){
		if(child->sibling==NULL){
			//Exp->ID
			Type value = getTable(child->value);  //判断是否定义过
			if(value==NULL||value->kind==FUNCTION){
				printf("Error type 1 at Line %d: Undefined variable \"%s\".\n", child->line, child->value);
				return NULL;
			}

			//--------------------------translate------------
			if(place!=NULL){
				place->kind = VARIABLE;
				// place->u.value = child->value;
				setOpValue(place, child->value);
			}
			//--------------------------translate------------
			value->assign = BOTH;
			return value;
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
				else{
					//--------------------------translate----
					if(strcmp(func->name,"read")==0){
						if(place!=NULL){
							InterCode funcCode = malloc(sizeof(InterCode_));
							memset(funcCode, 0, sizeof(InterCode_));
							funcCode->kind = READ_N;
							funcCode->u.sinop.op = place;
							insertCode(funcCode);
						}
					}
					else{
						Operand funcOp = malloc(sizeof(Operand_));
						memset(funcOp, 0, sizeof(Operand_));
						funcOp->kind = FUNCTION;
						funcOp->u.value = func->name;
						if(place!=NULL){
							InterCode funcCode = malloc(sizeof(InterCode_));
							memset(funcCode, 0, sizeof(InterCode_));
							funcCode->kind = CALL_N;
							funcCode->u.assign.left = place;
							funcCode->u.assign.right = funcOp;
							insertCode(funcCode);
						}
						else{
							Operand uselessOp = malloc(sizeof(Operand_));
							memset(uselessOp, 0, sizeof(Operand_));
							uselessOp->kind = TEMPVAR;
							uselessOp->u.var_no = temVarNo;
							temVarNo++;

							InterCode funcCode = malloc(sizeof(InterCode_));
							memset(funcCode, 0, sizeof(InterCode_));
							funcCode->kind = CALL_N;
							funcCode->u.assign.left = uselessOp;
							funcCode->u.assign.right = funcOp;
							insertCode(funcCode);
						}
					}
					//--------------------------translate----
				}
			}
			else{

				//--------------------------translate--------
					Operand argsListHead =  NULL;
				//--------------------------translate--------

				if(Args(child,param)!=0){  //比较两个类型是否匹配
					printf("Error type 9 at Line %d: Function \"%s\" is not applicable for arguments.\n", child->line, func->u.function->name);
					return NULL;
				}
				else{
					//--------------------------translate----
					if(strcmp(func->name,"write")==0){
						InterCode funcCode = malloc(sizeof(InterCode_));
						memset(funcCode, 0, sizeof(InterCode_));
						funcCode->kind = WRITE_N;
						assert(argsListHead!=NULL);
						funcCode->u.sinop.op = argsListHead;
						insertCode(funcCode);
					}
					else{
						Operand argsP = argsListHead;
						while(argsP!=NULL){
							// PARAM argsP
							//直接按照队列方向即从右向左入栈
							InterCode argCode = malloc(sizeof(InterCode_));
							memset(argCode, 0, sizeof(InterCode_));
							argCode->kind = ARG_N;
							argCode->u.sinop.op = argsP;
							insertCode(argCode);
							argsP = argsP->next;
						}
						Operand funcOp = malloc(sizeof(Operand_));
						memset(funcOp, 0, sizeof(Operand_));
						funcOp->kind = FUNCTION;
						funcOp->u.value = func->name;
						if(place!=NULL){
							InterCode funcCode = malloc(sizeof(InterCode_));
							memset(funcCode, 0, sizeof(InterCode_));
							funcCode->kind = CALL_N;
							funcCode->u.assign.left = place;
							funcCode->u.assign.right = funcOp;
							insertCode(funcCode);
						}
						else{
							Operand uselessOp = malloc(sizeof(Operand_));
							memset(uselessOp, 0, sizeof(Operand_));
							uselessOp->kind = TEMPVAR;
							uselessOp->u.var_no = temVarNo;
							temVarNo++;

							InterCode funcCode = malloc(sizeof(InterCode_));
							memset(funcCode, 0, sizeof(InterCode_));
							funcCode->kind = CALL_N;
							funcCode->u.assign.left = uselessOp;
							funcCode->u.assign.right = funcOp;
							insertCode(funcCode);
						}
					}
					//--------------------------translate----
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

		//--------------------------translate-----------------
			if(place!=NULL){
				place->kind = CONSTANT;
				place->u.value = child->value;
			}
		//--------------------------translate-----------------

		return type;
	}
	else if(strcmp(child->identifier,"FLOAT")==0){
		//Exp->FLOAT
		Type type = (Type)malloc(sizeof(struct Type_));
		type->kind = BASIC;
		type->u.basic = TYPE_FLOAT;
		type->assign = RIGHT;

		//--------------------------translate-----------------
			if(place!=NULL){
				place->kind = CONSTANT;
				place->u.value = child->value;
			}
		//--------------------------translate-----------------

		return type;
	}
	else if(strcmp(child->identifier,"Exp")==0){
		//Exp->Exp ...
		if(strcmp(child->sibling->identifier,"ASSIGNOP")==0 ){
			//Exp->Exp ASSIGNOP Exp

			//--------------------------translate-------------
				Operand leftOp = malloc(sizeof(Operand_));
				memset(leftOp, 0, sizeof(Operand_));
				leftOp->kind = TEMPVAR;
				leftOp->u.var_no = temVarNo;
				temVarNo++;

				Operand rightOp = malloc(sizeof(Operand_));
				memset(rightOp, 0, sizeof(Operand_));
				rightOp->kind = TEMPVAR;
				rightOp->u.var_no = temVarNo;
				int rightOpNo = rightOp->u.var_no;
				temVarNo++;
			//--------------------------translate-------------

			//判断表达式左边是否为左值
			Type lhs = Exp(child,leftOp);
			Type rhs = Exp(child->sibling->sibling,rightOp);
			if(lhs==NULL || rhs==NULL)
				return NULL;
			if(lhs->assign==RIGHT){
				printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n", child->line);
				return NULL;
			}



			//判断赋值号两边表达式是否类型匹配
			if(typeEqual(lhs, rhs)==0){

				//--------------------------translate---------
					if( !(rightOp->kind==TEMPVAR && rightOp->u.var_no==rightOpNo && (leftOp->kind==TEMPVAR || leftOp->kind==VARIABLE)) ){
						//rightOp被改 说明需要生成赋值语句
						// leftOp := rightOp
						InterCode assignCode1 = malloc(sizeof(InterCode_));
						memset(assignCode1, 0, sizeof(InterCode_));
						assignCode1->kind = ASSIGN_N;
						assignCode1->u.assign.left = leftOp;
						assignCode1->u.assign.right = rightOp;
						insertCode(assignCode1);
					}
					else{
						//否则将rightOp替换为leftOp即可
						memcpy(rightOp, leftOp, sizeof(Operand_));
					}
					if(place!=NULL){
						//place := rightOp
						InterCode assignCode2 = malloc(sizeof(InterCode_));
						memset(assignCode2, 0, sizeof(InterCode_));
						assignCode2->kind = ASSIGN_N;
						assignCode2->u.assign.left = place;
						assignCode2->u.assign.right = rightOp;
						insertCode(assignCode2);
					}
				//--------------------------translate---------

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

			//--------------------------translate-------------
				Operand leftOp = malloc(sizeof(Operand_));
				memset(leftOp, 0, sizeof(Operand_));
				leftOp->kind = TEMPVAR;
				leftOp->u.var_no = temVarNo;
				temVarNo++;
				Operand rightOp = malloc(sizeof(Operand_));
				memset(rightOp, 0, sizeof(Operand_));
				rightOp->kind = TEMPVAR;
				rightOp->u.var_no = temVarNo;
				temVarNo++;
			//--------------------------translate-------------

			Type lhs = Exp(child,leftOp);
			Type rhs = Exp(child->sibling->sibling,rightOp);



			if(lhs==NULL||rhs==NULL)
				return NULL;
			if(lhs->kind==BASIC && rhs->kind==BASIC &&
							lhs->u.basic==rhs->u.basic){

				//--------------------------translate---------
				if(place!=NULL){
					InterCode calcCode = malloc(sizeof(InterCode_));
					memset(calcCode, 0, sizeof(InterCode_));
					if(strcmp(child->sibling->identifier,"PLUS")==0)
						calcCode->kind=ADD_N;
					else if(strcmp(child->sibling->identifier,"MINUS")==0)
						calcCode->kind=SUB_N;
					else if(strcmp(child->sibling->identifier,"STAR")==0)
						calcCode->kind=MUL_N;
					else if(strcmp(child->sibling->identifier,"DIV")==0)
						calcCode->kind=DIV_N;
					else
						assert(0);
					calcCode->u.binop.result = place;
					calcCode->u.binop.op1 = leftOp;
					calcCode->u.binop.op2 = rightOp;
					insertCode(calcCode);
				}
				//--------------------------translate---------

				lhs->assign = RIGHT;
				return lhs;
			}
			else{
				printf("Error type 7 at Line %d: Type mismatched for operands.\n", child->line);
				return NULL;
			}
		}
		else if(strcmp(child->sibling->identifier,"LB")==0){

			//--------------------------translate-------------
				//用于存储基地址
				Operand baseOp = malloc(sizeof(Operand_));
				memset(baseOp, 0, sizeof(Operand_));
				baseOp->kind = TEMPVAR;
				baseOp->u.var_no = temVarNo;
				temVarNo++;
			//--------------------------translate-------------

			//Exp->Exp LB Exp RB
			Type array=Exp(child,baseOp);

			if(array==NULL)
				return NULL;
			if(array->kind!=ARRAY){
				printf("Error type 10 at Line %d: \"%s\" is not an array.\n", child->line, child->child->value);
				return NULL;
			}

			//--------------------------translate-------------
				int subscipt = 1;
				if(strcmp(child->sibling->sibling->child->identifier, "INT")==0)
					subscipt = atoi(child->sibling->sibling->child->value);
				Operand subscriptOp=NULL;
				if(subscipt!=0){
					//用于存储数组下标
					subscriptOp = malloc(sizeof(Operand_));
					memset(subscriptOp, 0, sizeof(Operand_));
					subscriptOp->kind = TEMPVAR;
					subscriptOp->u.no = temVarNo;
					temVarNo++;
				}
			//--------------------------translate-------------

			child = child->sibling->sibling;
			Type arrayNumber = Exp(child,subscriptOp);
			if(arrayNumber==NULL)
				return NULL;
			if(arrayNumber->kind!=BASIC || arrayNumber->u.basic!=TYPE_INT){
				printf("Error type 12 at Line %d: \"%s\" is not an integer.\n", child->line , child->child->value);
				return NULL;
			}

			//--------------------------translate-------------
				//用于存储计算出的偏移量
				Operand offsetOp = malloc(sizeof(Operand_));
				memset(offsetOp, 0, sizeof(Operand_));
				offsetOp->kind = TEMPVAR;
				offsetOp->u.var_no = temVarNo;
				temVarNo++;
				if(subscipt!=0){
				//用于存储宽度
					Operand wideOp = malloc(sizeof(Operand_));
					memset(wideOp, 0, sizeof(Operand_));
					wideOp->kind = CONST_OP;
					char* wideStr = malloc(16*sizeof(char));
					int wide = array->u.array.size;
					sprintf(wideStr, "%d", wide);
					wideOp->u.value = wideStr;

					//offsetOp := subscriptOp MUL wideOp
					InterCode offsetCode = malloc(sizeof(InterCode_));
					memset(offsetCode, 0, sizeof(InterCode_));
					offsetCode->kind = MUL_N;
					offsetCode->u.binop.result = offsetOp;
					offsetCode->u.binop.op1 = subscriptOp;
					offsetCode->u.binop.op2 = wideOp;
					insertCode(offsetCode);

					// 地址 := baseOp ADD offsetOp
					InterCode addrCode = malloc(sizeof(InterCode_));
					memset(addrCode, 0, sizeof(InterCode_));
					addrCode->kind = ADD_CODE;
					addrCode->u.binop.op1 = baseOp;
					addrCode->u.binop.op2 = offsetOp;
					if(arraySym->u.array.elem->kind==BASIC){
						//如果下一层是BASIC 则place是地址指向的位置
						Operand temAddrOp = malloc(sizeof(Operand_));
						memset(temAddrOp, 0, sizeof(Operand_));
						temAddrOp->kind = TEMPVAR;
						temAddrOp->u.no = temVarNo;
						temVarNo++;

						addrCode->u.binop.result = temAddrOp;
						place->kind = ADDR_OP;
						place->u.addr = temAddrOp;
					}
					else{
						//如果下一层仍然是数组 则place是地址
						addrCode->u.binop.result = place;
					}
					insertCode(addrCode);
				}
				else{
					// 地址 := baseOp
					InterCode addrCode = malloc(sizeof(InterCode_));
					memset(addrCode, 0, sizeof(InterCode_));
					addrCode->kind = ASSIGN_N;
					addrCode->u.assign.right = baseOp;
					if(arraySym->u.array.elem->kind==BASIC){
						//如果下一层是BASIC 则place是地址指向的位置
						Operand temAddrOp = malloc(sizeof(Operand_));
						memset(temAddrOp, 0, sizeof(Operand_));
						temAddrOp->kind = TEMPVAR;
						temAddrOp->u.var_no = temVarNo;
						temVarNo++;

						addrCode->u.assign.left = temAddrOp;
						place->kind = TADDRESS;
						place->u.addr = temAddrOp;
					}
					else{
						//如果下一层仍然是数组 则place是地址
						addrCode->u.assign.left = place;
					}
					insertCode(addrCode);
				}
			//--------------------------translate-------------

			Type rtn = (Type)malloc(sizeof(struct Type_));
			memcpy(rtn, array->u.array.elem, sizeof(struct Type_));
			rtn->assign = BOTH;
			return rtn;
		}
		else if(strcmp(child->sibling->identifier,"DOT")==0){

			//--------------------------translate-------------
				Operand strucVarOp = malloc(sizeof(Operand_));
				memset(strucVarOp, 0, sizeof(Operand_));
				strucVarOp->kind = TEMPVAR;
				strucVarOp->u.var_no = temVarNo;
				temVarNo++;
			//--------------------------translate-------------

			//Exp->Exp DOT ID
			Type structure = Exp(child,strucVarOp);
			if(structure==NULL)
				return NULL;
			if(structure->kind!=STRUCTURE){
				printf("Error type 13 at Line %d: Illegal use of \".\".\n", child->line);
				return NULL;
			}
			FieldList structDomain = structure->u.structure->domain;
			child = child->sibling->sibling;

			int offset=0;

			while(structDomain!=NULL){
				if(strcmp(structDomain->name, child->value) ==0){

					//--------------------------translate-----
					if(offset==0){
						if(place!=NULL){
							if(structDomain->kind==BASIC){
								place->kind = TADDRESS;
								place->u.addr = strucVarOp;
							}
							else{
								memcpy(place, strucVarOp, sizeof(Operand_));
							}
						}
					}
					else{
						//用于存储偏移量
						Operand offsetOp = malloc(sizeof(Operand_));
						memset(offsetOp, 0, sizeof(Operand_));
						offsetOp->kind = CONSTANT;
						char* offsetStr = malloc(16*sizeof(char));
						sprintf(offsetStr, "%d", offset);
						offsetOp->u.value = offsetStr;
						// 地址 := strucVarOp ADD offsetOp
						InterCode addrCode = malloc(sizeof(InterCode_));
						memset(addrCode, 0, sizeof(InterCode_));
						addrCode->kind = ADD_N;
						addrCode->u.binop.op1 = strucVarOp;
						addrCode->u.binop.op2 = offsetOp;
						if(strucField->kind==BASIC){
							//如果下一层是BASIC 则place是地址指向的位置
							Operand temAddrOp = malloc(sizeof(Operand_));
							memset(temAddrOp, 0, sizeof(Operand_));
							temAddrOp->kind = TEMPVAR;
							temAddrOp->u.var_no = temVarNo;
							temVarNo++;

							addrCode->u.binop.result = temAddrOp;
							place->kind = TADDRESS;
							place->u.addr = temAddrOp;
						}
						else{
							//如果下一层仍然是struct 则place是地址
							addrCode->u.binop.result = place;
						}
						insertCode(addrCode);
					}
					//--------------------------translate-----

					Type rtn = (Type)malloc(sizeof(struct Type_));
					memcpy(rtn, structDomain->type, sizeof(struct Type_));
					rtn->assign = BOTH;
					return rtn;
				}
				structDomain = structDomain->tail;  //可能有错，找下一个

				offset += sizeofType(strucField);

			}
			printf("Error type 14 at Line %d: Non-existent field \"%s\".\n", child->line, child->value);
			return NULL;
		}

		return NULL;
	}

	return NULL;
}

//exp condition
Type Exp_Cond(Node *n,Operand label_true,Operand label_false)
{//printName(n->name);
	Node *child = n->children;
	Type type;
	if(strcmp(child->name,"Exp")==0)
	{
		Node *child2=child->next;
		if(strcmp(child2->name,"RELOP")==0)//< >
		{
			//new temp
			Operand t1=malloc(sizeof(Operand_));
			t1->kind=TEMPVAR;
			t1->u.var_no=temVarNo++;
			Operand t2=malloc(sizeof(Operand_));
			t2->kind=TEMPVAR;
			t2->u.var_no=temVarNo++;

			Node* child3=child2;
			child2=child2->next;
			Type tp=Exp(child,t1);	//code1
			Type tp2=Exp(child2,t2);	//code2
			if(tp==NULL||tp2==NULL)return NULL;
			else if((tp->kind==BASIC||tp->kind==FUNCTION)&&(tp2->kind==BASIC||tp2->kind==STRUCTURE)&&tp->u.basic==tp2->u.basic)
			{
				InterCode code3=malloc(sizeof(nterCode_));
				code3->kind=IFGOTO_N;
				code3->u.triop.t1=t1;
				code3->u.triop.op=child3->value;
				code3->u.triop.t2=t2;
				code3->u.triop.label=label_true;
				insertCode(code3);		//code3

				InterCode gotolbf=malloc(sizeof(InterCode_));
				gotolbf->kind=GOTO_N;
				gotolbf->u.one.op=label_false;
				insertCode(gotolbf);		//goto label false
				return tp;
			}
			else
			{
				printf("Error type 7 at line %d: Operands type mismatched!\n",child->row);
				return NULL;
			}

		}
		else if(strcmp(child2->name,"AND")==0)
		{
			//new temp
			Operand lb1=malloc(sizeof(Operand_));
			lb1->kind=LABEL;
			lb1->u.var_no=labelNo++;

			Type t=Exp_Cond(child,lb1,label_false);	//code1

			InterCode lb1code=malloc(sizeof(InterCode_));
			lb1code->kind=LABEL_N;
			lb1code->u.one.op=lb1;
			insertCode(lb1code);		//label 1

			child2=child2->next;
			Type t2=Exp_Cond(child2,label_true,label_false);	//code2
			if(t==NULL||t2==NULL)return NULL;
			else if((t->kind==BASIC||t->kind==FUNCTION)&&(t2->kind==BASIC||t2->kind==FUNCTION)&&t->u.basic==t2->u.basic)
								return t;
			else
			{
				printf("Error type 7 at line %d: Operands type mismatched!!\n",child->row);
				return NULL;
			}

		}
		else if(strcmp(child2->name,"OR")==0)
		{
			//new temp
			Operand lb1=malloc(sizeof(Operand_));
			lb1->kind=LABEL;
			lb1->u.var_no=labelNo++;

			child2=child2->next;
			Type t=Exp_Cond(child,label_true,lb1);	//code1

			InterCode lb1code=malloc(sizeof(InterCode_));
			lb1code->kind=LABEL_N;
			lb1code->u.one.op=lb1;
			insertCode(lb1code);		//label 1

			Type t2=Exp_Cond(child2,label_true,label_false);	//code2
			if(t==NULL||t2==NULL)return NULL;
			else if((t->kind==BASIC||t->kind==FUNCTION)&&(t2->kind==BASIC||t2->kind==FUNCTION)&&t->u.basic==t2->u.basic)
				return t;
			else
			{
				printf("Error type 7 at line %d: Operands type mismatched!!!\n",child->row);
				return NULL;
			}

		}

	}
	if(strcmp(child->name,"NOT")==0)	//not
	{
		child=child->next;
		Type t=Exp_Cond(child,label_false,label_true);
		if(t==NULL)return NULL;
		if(t->kind==BASIC&&t->u.basic==TYPE_INT)return t;
		printf("Error type 7 at line %d: Operands type mismatched\n",child->row);
		return NULL;
	}
	Operand t1=malloc(sizeof(Operand_));
	t1->kind=TEMPVAR;
	t1->u.var_no=temVarNo++;
	type=Exp(n,t1);		//code1
	InterCode code2=malloc(sizeof(InterCode_));
	code2->kind=IFGOTO_K;
	code2->u.triop.t1=t1;
	code2->u.triop.op=malloc(32);
	strcpy(code2->u.triop.op,"!=");
	Operand t2=malloc(sizeof(Operand_));
	t2->kind=CONSTANT;
	t2->u.value=zeroStr;
	code2->u.triop.t2=t2;
	code2->u.triop.label=label_true;
	insertCode(code2);		//code2

	InterCode gotolbf=malloc(sizeof(InterCode_));
	gotolbf->kind=GOTO_N;
	gotolbf->u.sinop.op=label_false;
	insertCode(gotolbf);		//goto label false
	return type;
}

int Args(Node *n, FieldList param){
	if(n==NULL)
		return 1;
	printf("%s\n",n->identifier);

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
