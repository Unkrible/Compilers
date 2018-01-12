#include "mips.h"

void printMips(char *fileName){
	fp = fopen(fileName, "w");
	if(fp==NULL){
		printf("ERROR: Can not open file \"%s\".", fileName);
		return;
	}
	initRegs();
	fputs(".data\n", fp);
	fputs("_prompt: .asciiz \"Enter an integer:\"\n", fp);
	fputs("_ret: .asciiz \"\\n\"\n", fp);
	fputs(".globl main\n", fp);
	fputs(".text\n",fp);
	// FUNCTION read
	fputs("\nread:\n", fp);
	fputs("\tli $v0, 4\n", fp);
	fputs("\tla $a0, _prompt\n",fp);
	fputs("\tsyscall\n", fp);
	fputs("\tli $v0, 5\n", fp);
	fputs("\tsyscall\n", fp);
	fputs("\tjr $ra\n", fp);
	// FUNCTION write
	fputs("\nwrite:\n",fp);
	fputs("\tli $v0, 1\n", fp);
	fputs("\tsyscall\n", fp);
	fputs("\tli $v0, 4\n", fp);
	fputs("\tla $a0, _ret\n", fp);
	fputs("\tsyscall\n", fp);
	fputs("\tmove $v0, $0\n", fp);
	fputs("\tjr $ra\n\n", fp);

	InterCode itor = code_head;
	while(itor != NULL){
		printMipsCode(itor);
		itor = itor->next;
	}
}

void printMipsCode(InterCode interCode){
		printf("%d\n", interCode->kind);
	switch (interCode->kind) {
		case LABEL_N:
			mipsLabel(interCode);
			break;
		case ASSIGN_N:
			mipsAssign(interCode);
			break;
		case ADD_N: case SUB_N: case MUL_N: case DIV_N:
			mipsOperation(interCode);
			break;
		case READ_N:
			mipsRead(interCode);
			break;
		case WRITE_N:
			mipsWrite(interCode);
			break;
		case CALL_N:
			mipsCall(interCode);
			break;
		case RETURN_N:
			mipsReturn(interCode);
			break;
		case GOTO_N:
			mipsGOTO(interCode);
			break;
		case IFGOTO_N:
			mipsIFGOTO(interCode);
			break;
		case FUNCTION_N:
			mipsFunction(interCode);
			break;
		case ARG_N:
			mipsArg(interCode);
			break;
		case PARAM_N:
			mipsParam(interCode);
			break;
		case DEC_N:
			mipsDec(interCode);
			break;
		default:
			printf("Error: Unknown Kind to MIPS\n");
			exit(-1);
	}
}

void mipsLabel(InterCode interCode){
	char str[STR_LENGTH];
	memset(str, 0, sizeof(str));
	// x:
	sprintf(str, "label%d:\n",interCode->u.sinop.op->u.var_no);
	fputs(str, fp);
}

void mipsAssign(InterCode interCode){
	char str[STR_LENGTH];
	memset(str, 0, sizeof(str));
	Operand leftOp = interCode->u.assign.left;
	Operand rightOp = interCode->u.assign.right;
	int x, y;
	if(leftOp->kind==TEMPVAR || leftOp->kind==VARIABLE){
		// x:= #k
		if(rightOp->kind == CONSTANT){
			// li reg(x), k
				x = getReg(leftOp);
				sprintf(str, "\tli %s, %s\n", printReg(x), rightOp->u.value);
				fputs(str, fp);
		}
		// x:= y
		else if(rightOp->kind==TEMPVAR || rightOp->kind==VARIABLE){
				x = getReg(leftOp);
				y = getReg(rightOp);
				sprintf(str, "\tmove %s, %s\n", printReg(x), printReg(y));
				fputs(str, fp);
		}
		// x:= *y
		else if(rightOp->kind==TADDRESS || rightOp->kind==VADDRESS){
				x = getReg(leftOp);
				y = getReg(rightOp);
				sprintf(str, "\tlw %s, 0(%s)\n", printReg(x), printReg(y));
				fputs(str, fp);
		}

	} else if(leftOp->kind==TADDRESS || leftOp->kind==VADDRESS){
		// *x = y
		if(rightOp->kind == CONSTANT){
			// Can't resolve it temporarily
			exit(-1);
		} else if(rightOp->kind==VARIABLE || rightOp->kind==TEMPVAR){
			x = getReg(leftOp);
			y = getReg(rightOp);
			sprintf(str, "\tsw %s, 0(%s)\n", printReg(y), printReg(x));
			fputs(str, fp);
		}
	}
	swReg(x);
}

void mipsOperation(InterCode interCode){
	char str[STR_LENGTH];
	memset(str, 0, sizeof(str));
	// + - * /
	Operand result = interCode->u.binop.result;
	Operand leftOp = interCode->u.binop.op1;
	Operand rightOp = interCode->u.binop.op2;
	int x, y, z;
	if((leftOp->kind==TEMPVAR||leftOp->kind==VARIABLE)&&(rightOp->kind==TEMPVAR||rightOp->kind==VARIABLE)){
		x = getReg(result);
		y = getReg(leftOp);
		z = getReg(rightOp);
		switch (interCode->kind) {
			case ADD_N:
			sprintf(str, "\tadd %s, %s, %s\n", printReg(x), printReg(y), printReg(z));
			break;
			case SUB_N:
			sprintf(str, "\tsub %s, %s, %s\n", printReg(x), printReg(y), printReg(z));
			break;
			case MUL_N:
			sprintf(str, "\tmul %s, %s, %s\n", printReg(x), printReg(y), printReg(z));
			break;
			case DIV_N:
			sprintf(str, "\tdiv %s, %s\n\tmflo %s\n", printReg(y), printReg(z), printReg(x));
			break;
		}
		fputs(str, fp);
	}
	else if((leftOp->kind==CONSTANT)&&(rightOp->kind==TEMPVAR||rightOp->kind==VARIABLE)){
		x = getReg(result);
		y = getReg(rightOp);
		switch (interCode->kind) {
			case ADD_N:
				sprintf(str, "\taddi %s, %s, %s\n", printReg(x), printReg(y), leftOp->u.value);
				break;
			case SUB_N:
			sprintf(str, "\taddi %s, %s, -%s\n", printReg(x), printReg(y), leftOp->u.value);
				break;
			default:
				exit(-1);
		}
		fputs(str, fp);
	}
	else if((rightOp->kind==CONSTANT)&&(leftOp->kind==TEMPVAR||leftOp->kind==VARIABLE)){
		x = getReg(result);
		y = getReg(leftOp);
		switch (interCode->kind) {
			case ADD_N:
				sprintf(str, "\taddi %s, %s, %s\n", printReg(x), printReg(y), rightOp->u.value);
				break;
			case SUB_N:
			sprintf(str, "\taddi %s, %s, -%s\n", printReg(x), printReg(y), rightOp->u.value);
				break;
			case MUL_N: case DIV_N:
			default:
				exit(-1);
		}
		fputs(str, fp);
	}
	swReg(x);
}

void mipsRead(InterCode interCode){
	fputs("\tsubu $sp, $sp, 4\n", fp);
	fputs("\tsw $ra, 0($sp)\n", fp);

	int x = getReg(interCode->u.sinop.op);
	char str[STR_LENGTH];
	memset(str, 0, sizeof(str));
	sprintf(str, "\tjal read\n\tmove %s, $v0\n", printReg(x));
	fputs(str, fp);
	swReg(x);

	fputs("\tlw $ra, 0($sp)\n", fp);
	fputs("\taddi $sp, $sp, 4\n", fp);
}

void mipsWrite(InterCode interCode){
	fputs("\tsubu $sp, $sp, 4\n", fp);
	fputs("\tsw $ra, 0($sp)\n", fp);

	char str[STR_LENGTH];
	memset(str, 0, sizeof(str));
	int r = getReg(interCode->u.sinop.op);

	sprintf(str, "\tmove $a0, %s\n\tjal write\n", printReg(r));
	fputs(str, fp);
	swReg(r);

	fputs("\tlw $ra, 0($sp)\n", fp);
	fputs("\taddi $sp, $sp, 4\n", fp);
}

void mipsCall(InterCode interCode){
	fputs("\tsubu $sp, $sp, 4\n", fp);
	fputs("\tsw $ra, 0($sp)\n", fp);

	Operand op = interCode->u.assign.left;
	Operand func = interCode->u.assign.right;
	int x = getReg(op);
	// jal f
	// move reg(x), $v0
	char str[STR_LENGTH];
	memset(str, 0, sizeof(str));
	sprintf(str, "\tjal %s\n\tmove %s, $v0\n", func->u.value, printReg(x));
	fputs(str, fp);
	swReg(x);
	fputs("\tlw $ra, 0($sp)\n", fp);
	fputs("\taddi $sp, $sp, 4\n", fp);
}

void mipsReturn(InterCode interCode){
	char str[STR_LENGTH];
	memset(str, 0, sizeof(str));
	Operand op = interCode->u.sinop.op;
	if(op->kind!=CONSTANT){
		int x = getReg(op);
		// move $v0, reg(x)
		// jr $ra
		sprintf(str, "\tmove $v0, %s\n\taddi $sp, $sp, %d\n\tlw $fp, 0($sp)\n\taddi $sp, $sp, 4\n\tjr $ra\n", printReg(x), stackSize);
	} else {
		sprintf(str, "\tmove $v0, $%s\n\taddi $sp, $sp, %d\n\tlw $fp, 0($sp)\n\taddi $sp, $sp, 4\n\tjr $ra\n", op->u.value, stackSize);
	}
	fputs(str, fp);
}

void mipsGOTO(InterCode interCode){
	char str[STR_LENGTH];
	memset(str, 0, sizeof(str));
	// j x
	sprintf(str, "\tj label%d\n",interCode->u.sinop.op->u.var_no);
	fputs(str, fp);
}

void mipsIFGOTO(InterCode interCode){
	char str[STR_LENGTH];
	memset(str, 0, sizeof(str));
	Operand leftOp = interCode->u.triop.x;
	Operand rightOp = interCode->u.triop.y;
	Operand label = interCode->u.triop.label;
	char *op = interCode->u.triop.op;

	if(leftOp->kind != CONSTANT && rightOp->kind != CONSTANT){
		int x = getReg(leftOp);
		int y = getReg(rightOp);
		// bxx reg(x), reg(y), z
		if(strcmp(op, "==")==0){
			sprintf(str, "\tbeq %s, %s, label%d\n",printReg(x), printReg(y), label->u.var_no);
		} else if(strcmp(op, "!=")==0){
			sprintf(str, "\tbne %s, %s, label%d\n",printReg(x), printReg(y), label->u.var_no);
		} else if(strcmp(op, ">")==0){
			sprintf(str, "\tbgt %s, %s, label%d\n",printReg(x), printReg(y), label->u.var_no);
		} else if(strcmp(op, "<")==0){
			sprintf(str, "\tblt %s, %s, label%d\n",printReg(x), printReg(y), label->u.var_no);
		} else if(strcmp(op, ">=")==0){
			sprintf(str, "\tbge %s, %s, label%d\n",printReg(x), printReg(y), label->u.var_no);
		} else if(strcmp(op, "<=")==0){
			sprintf(str, "\tble %s, %s, label%d\n",printReg(x), printReg(y), label->u.var_no);
		}
	} else if(leftOp->kind == CONSTANT && rightOp->kind != CONSTANT){
		int y = getReg(rightOp);
		// bxx reg(x), reg(y), z
		if(strcmp(op, "==")==0){
			sprintf(str, "\tbeq %s, %s, label%d\n", printReg(y), leftOp->u.value, label->u.var_no);
		} else if(strcmp(op, "!=")==0){
			sprintf(str, "\tbne %s, %s, label%d\n", printReg(y), leftOp->u.value, label->u.var_no);
		} else if(strcmp(op, ">")==0){
			sprintf(str, "\tblt %s, %s, label%d\n", printReg(y), leftOp->u.value, label->u.var_no);
		} else if(strcmp(op, "<")==0){
			sprintf(str, "\tbgt %s, %s, label%d\n", printReg(y), leftOp->u.value, label->u.var_no);
		} else if(strcmp(op, ">=")==0){
			sprintf(str, "\tble %s, %s, label%d\n", printReg(y), leftOp->u.value, label->u.var_no);
		} else if(strcmp(op, "<=")==0){
			sprintf(str, "\tbge %s, %s, label%d\n", printReg(y), leftOp->u.value, label->u.var_no);
		}
	} else if(rightOp->kind == CONSTANT && leftOp->kind != CONSTANT){
		int x = getReg(leftOp);
		// bxx reg(x), reg(y), z
		if(strcmp(op, "==")==0){
			sprintf(str, "\tbeq %s, %s, label%d\n",printReg(x), rightOp->u.value, label->u.var_no);
		} else if(strcmp(op, "!=")==0){
			sprintf(str, "\tbne %s, %s, label%d\n",printReg(x), rightOp->u.value, label->u.var_no);
		} else if(strcmp(op, ">")==0){
			sprintf(str, "\tbgt %s, %s, label%d\n",printReg(x), rightOp->u.value, label->u.var_no);
		} else if(strcmp(op, "<")==0){
			sprintf(str, "\tblt %s, %s, label%d\n",printReg(x), rightOp->u.value, label->u.var_no);
		} else if(strcmp(op, ">=")==0){
			sprintf(str, "\tbge %s, %s, label%d\n",printReg(x), rightOp->u.value, label->u.var_no);
		} else if(strcmp(op, "<=")==0){
			sprintf(str, "\tble %s, %s, label%d\n",printReg(x), rightOp->u.value, label->u.var_no);
		}
	}
	fputs(str, fp);
}

void mipsFunction(InterCode interCode){

	char str[STR_LENGTH];
	memset(str, 0, sizeof(str));
	// function:
	sprintf(str, "%s:\n\tsubu $sp, $sp, 4\n\tsw $fp, 0($sp)\n\tmove $fp, $sp\n\tsubu $sp, $sp, %d\n",interCode->u.sinop.op->u.value,stackSize);
	fputs(str, fp);
	spOffset = 0;
	curParam = 0;
}

void mipsArg(InterCode interCode){
	char str[STR_LENGTH];
	memset(str, 0, sizeof(str));
	Operand op = interCode->u.sinop.op;
	Var_t *arg=NULL;
	if(op->kind == TEMPVAR){
		char argName[20];
		memset(argName, 0, 20);
		sprintf(argName, "t%d", op->u.var_no);
		arg = findVar(argName);
	} else if(op->kind ==VARIABLE){
		arg = findVar(op->u.value);
	}
	if(arg == NULL)
		exit(-1);
	sprintf(str, "\tlw $a%d, %d($fp)\n", curArg, arg->offset);
	fputs(str, fp);
	//TODO: add param if curArg>4
	++curArg;
	if(interCode->next==NULL || interCode->next->kind!=ARG_N){
		curArg = 0;
	}
}

void mipsParam(InterCode interCode){
	char str[STR_LENGTH];
	memset(str, 0, sizeof(str));

	Var_t* param = malloc(sizeof(Var_t));
	param->name = interCode->u.sinop.op->u.value;
	spOffset -= 4;
	param->offset = spOffset;
	addVar(param);

	sprintf(str, "\tsw $a%d, %d($fp)\n", curParam, param->offset);
	fputs(str, fp);
	//TODO: add param if curParam>4
	++curParam;
}

void mipsDec(InterCode interCode){

}

// Register
void initRegs(){
	int i=0;
	for(i=0; i<REG_NUM; i++){
		regs[i].name = regName[i];
		regs[i].var = NULL;
	}
}

int getReg(Operand op){
	char *name = NULL;
	if(op->kind == TEMPVAR){
		name = malloc(32);
		memset(name, 0, 32);
		sprintf(name, "t%d", op->u.var_no);
	} else if(op->kind == VARIABLE){
		name = op->u.value;
	}
	Var_t* var = findVar(name);
	int i = curReg + REG_T_START;
	curReg = (++curReg)%(REG_T_END - REG_T_START);
	if(var == NULL){
		var = malloc(sizeof(Var_t));
		var->name = name;
		spOffset -= 4;
		var->offset = spOffset;
		addVar(var);
		var->reg = i;
		regs[i].var = var;
	} else{
		var->reg = i;
		regs[i].var = var;
		lwReg(i, var);
	}
	return i;
}

char* printReg(int index){
	return regs[index].name;
}

void swReg(int index){
	char str[50];
	memset(str, 0, sizeof(str));
	Var_t *var = regs[index].var;
	sprintf(str, "\tsw %s, %d($fp)\n", printReg(index), var->offset);
	fputs(str, fp);
}

void lwReg(int index, Var_t *var){
	char str[50];
	memset(str, 0, sizeof(str));
	regs[index].var = var;
	sprintf(str, "\tlw %s, %d($fp)\n", printReg(index), var->offset);
	fputs(str, fp);
}

// Variable
void delVars(){
		Var_t *ptr = varList;
		while(ptr != NULL){
			varList = varList->next;
			free(ptr);
			ptr = varList;
		}
}

void addVar(Var_t *var){
	if(var==NULL)
		exit(-1);

	var->next = NULL;
	if(varList==NULL){
		varList = var;
	} else{
		Var_t *ptr = varList;
		while(ptr->next!=NULL)
			ptr = ptr->next;
		ptr->next = var;
	}
}

Var_t* findVar(char *name){
		Var_t *ptr = varList;
		while(ptr != NULL){
			if(strcmp(ptr->name, name)==0){
				break;
			}
			ptr = ptr->next;
		}
		return ptr;
}

Register_ regs[REG_NUM];
Var_t *varList = NULL;
FILE *fp = NULL;
int curReg = 0;
int spOffset = 0;
int curParam = 0;
int curArg = 0;
const int stackSize = 100;

char* regName[] = {
	"$zero",
	"$at",
	"$v0","$v1",
	"$a0","$a1","$a2","$a3",
	"$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7",
	"$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7",
	"$t8","$t9",
	"$k0","$k1",
	"$gp",
	"$sp",
	"$fp",
	"$ra"
};
