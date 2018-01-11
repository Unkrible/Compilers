#include "mips.h"



void printMips(char *fileName){
	fp = fopen(fileName, "w");
	if(fp==NULL){
		printf("ERROR: Can not open file 's'.", fileName);
		return;
	}

	fputs(".data\n_prompt: .asciiz \"Enter an integer:\"\n",fp);
	fputs("_ret: .asciiz \"\\n\"\n", fp)
	fputs(".globl main\n.text\n",fp);
	// FUNCTION read
	fputs("read:\n",fp);
	fputs("\tsubu $sp, $sp, 4\n",fp);
	fputs("\tsw $ra, 0($sp)\n",fp);
	fputs("\tsubu $sp, $sp, 4\n",fp);
	fputs("\tsw $fp, 0($sp)\n",fp);
	fputs("\taddi $fp, $sp, 8\n",fp);
	fputs("\tli $v0, 4\n\tla $a0, _prompt\n",fp);
	fputs("\tsyscall\n\tli $v0, 5\n\tsyscall\n",fp);
	fputs("\tsubu $sp, $fp, 8\n",fp);
	fputs("\tlw $fp, 0($sp)\n",fp);
	fputs("\taddi $sp, $sp, 4\n",fp);
	fputs("\tlw $ra, 0($sp)\n",fp);
	fputs("\taddi $sp, $sp, 4\n",fp);
	fputs("\tjr $ra\n",fp);
	// FUNCTION write
	fputs("\nwrite:\n",fp);
	fputs("\tsubu $sp, $sp, 4\n",fp);
	fputs("\tsw $ra, 0($sp)\n",fp);
	fputs("\tsubu $sp, $sp, 4\n",fp);
	fputs("\tsw $fp, 0($sp)\n",fp);
	fputs("\taddi $fp, $sp, 8\n",fp);
	fputs("\tli $v0, 1\n\tsyscall\n\tli $v0, 4\n",fp);
	fputs("\tla $a0, _ret\n\tsyscall\n\tmove $v0, $0\n",fp);
	fputs("\tsubu $sp, $fp, 8\n",fp);
	fputs("\tlw $fp, 0($sp)\n",fp);
	fputs("\taddi $sp, $sp, 4\n",fp);
	fputs("\tlw $ra, 0($sp)\n",fp);
	fputs("\taddi $sp, $sp, 4\n",fp);
	fputs("\tjr $ra\n",fp);

	InterCode itor = code_head;
	while(itor != NULL){
		printMipsCode(itor);
		itor = itor->next;
	}
}

void printMipsCode(InterCode interCode){
	switch (interCode->kind) {
		case LABEL_N:
		//TODO: write all the last label's variables to disk
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
			printf("Error: Unknown Kind to MIPS\n", );
			exit(-1);
	}
}

void mipsLabel(InterCode interCode){
	char str[50];
	memset(str, 0, sizeof(str));
	// x:
	sprintf(str, "label%d:\n",interCode->u.sinop.op->u.var_no);
	fputs(str, fp);
}

void mipsAssign(InterCode interCode, FILE fp){
	char str[50];
	memset(str, 0, sizeof(str));
	Operand leftOp = interCode->u.assign.left;
	Operand rightOp = interCode->u.assign.right;
	if(leftOp->kind==TEMPVAR || leftOp->kind==VARIABLE){
		// x:= #k
		if(rightOp->kind == CONSTANT){
			// li reg(x), k
				int i = getReg(leftOp);
				sprintf(str, "li %s, %s\n", printReg(i), rightOp->u.value);
				fputs(str, fp);
		}
		// x:= y
		else if(rightOp->kind==TEMPVAR || rightOp->kind==VARIABLE){
				int x = getReg(leftOp);
				int y = getReg(rightOp);
				sprintf(str, "move %s, %s\n", printReg(x), printReg(y));
				fputs(str, fp);
		}
		// x:= *y
		else if(rightOp->kind==TADDRESS || rightOp->kind==VADDRESS){
				int x = getReg(leftOp);
				int y = getReg(rightOp);
				sprintf(str, "lw %s, 0(%s)\n", printReg(x), printReg(y));
				fputs(str, fp);
		}

	} else if(leftOp->kind==TADDRESS || leftOp->kind==VADDRESS){
		// *x = y
		if(rightOp->kind == CONSTANT){
			// Can't resolve it temporarily
			exit(-1);
		} else if(rightOp->kind==VARIABLE || rightOp->kind==TEMPVAR){
			int x = getReg(leftOp);
			int y = getReg(rightOp);
			sprintf(str, "sw %s, 0(%s)\n", printReg(y), printReg(x));
			fputs(str, fp);
		}
	}
}

void mipsOperation(InterCode interCode){
	char str[50];
	memset(str, 0, sizeof(str));
	// + - * /
	Operand result = interCode->binop.result;
	Operand leftOp = interCode->binop.op1;
	Operand rightOp = interCode->binop.op2;

	if((leftOp->kind==TEMPVAR||leftOp->kind==VARIABLE)&&(rightOp->kind==TEMPVAR||rightOp->kind==VARIABLE)){
		int x = getReg(result);
		int y = getReg(leftOp);
		int z = getReg(rightOp);
		switch (interCode->kind) {
			case ADD_N:
			sprintf(str, "add %s, %s, %s\n", printReg(x), printReg(y), printReg(z));
			break;
			case SUB_N:
			sprintf(str, "sub %s, %s, %s\n", printReg(x), printReg(y), printReg(z));
			break;
			case MUL_N:
			sprintf(str, "mul %s, %s, %s\n", printReg(x), printReg(y), printReg(z));
			break;
			case DIV_N:
			sprintf(str, "div %s, %s\nmflo %s\n", printReg(y), printReg(z), printReg(,));
			break;
		}
		fputs(str, fp);
	}
	else if((leftOp->kind==CONSTANT)&&(rightop->kind==TEMPVAR||rightOp->kind==VARIABLE)){
		int x = getReg(result);
		int y = getReg(rightOp);
		switch (interCode->kind) {
			case ADD_N:
				sprintf(str, "addi %s, %s, %s\n", printReg(x), printReg(y), leftOp->u.value);
				break;
			case SUB_N:
			sprintf(str, "addi %s, %s, -%s\n", printReg(x), printReg(y), leftOp->u.value);
				break;
			default:
				exit(-1);
		}
		fputs(str, fp);
	}
	else if((rightOp->kind==CONSTANT)&&(leftOp->kind==TEMPVAR||leftOp->kind==VARIABLE)){
		int x = getReg(result);
		int y = getReg(leftOp);
		switch (interCode->kind) {
			case ADD_N:
				sprintf(str, "addi %s, %s, %s\n", printReg(x), printReg(y), rightOp->u.value);
				break;
			case SUB_N:
			sprintf(str, "addi %s, %s, -%s\n", printReg(x), printReg(y), rightOp->u.value);
				break;
			case MUL_N: case DIV_N:
			default:
				exit(-1);
		}
		fputs(str, fp);
	}
}

void mispRead(InterCode interCode){
	char str[50];
	memset(str, 0, sizeof(str));
	r = getReg(interCode->u.signop.op);

	sprintf(str, "jal read\nmove %s, $v0", printReg(r));
	fputs(str, fp);
}

void mispWrite(InterCode interCode){

}

void mispCall(InterCode interCode){
	char str[50];
	memset(str, 0, sizeof(str));
	Operand op = interCode->u.assign.left;
	Operand func = interCode->u.assign.right;
	int x = getReg(op);
	// jal f
	// move reg(x), $v0
	sprintf(str, "jal %s\nmove %s, $v0\n",op->u.value,printReg(x));
	fputs(str, fp);
}

void mispReturn(InterCode interCode){
	char str[50];
	memset(str, 0, sizeof(str));
	Operand op = interCode->u.sinop.op;
	int x = getReg(op);
	// move $v0, reg(x)
	// jr $ra
	sprintf(str, "move $v0, %s\njr $ra\n",printReg(x));
	fputs(str, fp);
}

void mispGOTO(InterCode interCode){
	char str[50];
	memset(str, 0, sizeof(str));
	// j x
	sprintf(str, "j label%d:\n",interCode->u.sinop.op->u.var_no);
	fputs(str, fp);
}

void mispIFGOTO(InterCode interCode){
	char str[50];
	memset(str, 0, sizeof(str));
	Operand leftOp = interCode->u.triop.x;
	Operand rightOp = interCode->u.triop.y;
	Operand label = interCode->u.triop.label;
	char *op = interCode->u.triop.op;

	int x = getReg(leftOp);
	int y = getReg(rightOp);
	// bxx reg(x), reg(y), z
	if(strcmp(op, "==")==0){
		sprintf(str, "beq %s, %s, label%d\n",printReg(x), printReg(y), label->u.var_no);
	} else if(strcmp(op, "!=")==0){
		sprintf(str, "bne %s, %s, label%d\n",printReg(x), printReg(y), label->u.var_no);
	} else if(strcmp(op, ">")==0){
		sprintf(str, "bgt %s, %s, label%d\n",printReg(x), printReg(y), label->u.var_no);
	} else if(strcmp(op, "<")==0){
		sprintf(str, "blt %s, %s, label%d\n",printReg(x), printReg(y), label->u.var_no);
	} else if(strcmp(op, ">=")==0){
		sprintf(str, "bge %s, %s, label%d\n",printReg(x), printReg(y), label->u.var_no);
	} else if(strcmp(op, "<=")==0){
		sprintf(str, "ble %s, %s, label%d\n",printReg(x), printReg(y), label->u.var_no);
	}
	fputs(str, fp);
}

void mipsFunction(InterCode interCode){
	char str[50];
	memset(str, 0, sizeof(str));
	// function:
	sprintf(str, "%s:\n",interCode->u.sinop.op->u.value);
	spOffset = 0;
	fputs(str, fp);
}

void mipsArg(InterCode interCode){
	char str[50];
	memset(str, 0, sizeof(str));
	Operand op = interCode->u.sinop;
	char argName[20];
	memset(argName, 0, 20);
	sprintf(argName, "t%d", op->u.var_no);
	Var_t *arg = findVar(argName);
	if(arg == NULL)
		exit(-1);
	sprintf(str, "lw $a%d, %d($fp)\n", curParam, arg->offset);
	fputs(str, fp);
	++curParam;
	if(interCode->next==NULL || interCode->next->kind!=ARG_N){
		curParam = 0;
	}
}

void mipsParam(InterCode interCode){
	char str[50];
	memset(str, 0, sizeof(str));

	Var_t* param = malloc(sizeof(Var_t));
	param->name = malloc(sizeof(32));
	memset(param, 0, sizeof(param->name));
	sprintf(param->name, "v%d", interCode->u.sinop.op->u.var_no);
	spOffset -= 4;
	param->offset = spOffset;
	addVar(param);

	sprintf(str, "sw $a%d, %d($fp)\n", curParam, param->offset);
	fputs(str, fp);

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
		offset -= 4;
		var->offset = offset;
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
	return regs[i].name;
}

void swReg(int index){
	char str[50];
	memset(str, 0, sizeof(str));
	Var_t *var = regs[index].var;
	sprintf(str, "sw %s, %d($fp)\n", printReg(index), var->offset);
	fputs(str, fp);
}

void lwReg(int index, Var_t *var){
	char str[50];
	memset(str, 0, sizeof(str));
	regs[index].var = var;
	sprintf(str, "lw %s, %d($fp)\n", printReg(index), var->offset);
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
		}
		return ptr;
}

Register_ regs[REG_NUM];
Var_t *varList = NULL;
FILE *fp = NULL;
int curReg = 0;
int spOffset = 0;
int curParam = 0;

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
