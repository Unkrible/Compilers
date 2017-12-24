#ifndef _INTERCODE_H
#define _INTERCODE_H

typedef struct Operand_* Operand;
typedef struct InterCode_* InterCode;
typedef struct Label_No_* Label_No;


typedef struct Operand_{
	enum{TEMPVAR,VARIABLE,CONSTANT,VADDRESS,LABEL,FUNCTION_,TADDRESS }kind;

	union{
		int var_no;
		char* value;
		Operand addr;
	}u;
	Operand next;
} Operand_;

typedef struct InterCode_{
	enum{ASSIGN_N,ADD_N,SUB_N,MUL_N,DIV_N,RETURN_N,LABEL_N,GOTO_N,IFGOTO_N,READ_N,WRITE_N,CALL_N,ARG_N,FUNCTION_N,PARAM_N,DEC_N,ADDRESS_N}kind;

	union{
		struct{	Operand op;	}sinop;

		struct{ Operand left,right; }assign;

		struct{ Operand op1,op2,result; }binop;

		struct{ Operand x; Operand y; Operand label; char *op; }triop;

		struct{ Operand op; int size; }dec;
	}u;

	InterCode pre;
	InterCode next;
} InterCode_;

typedef struct Label_No_
{
	int no;
	Label_No next;
} Label_No_;

void insertCode(InterCode c);
void deleteCode(InterCode c);
void printCode(char *fileName);
void printOp(Operand p,FILE *fp);

extern InterCode code_head,code_tail;
extern char* zeroStr;
extern char* oneStr;
extern char* neStr;
extern int temVarNo;
extern int labelNo;
extern int varNo;

void optimizeGoto();
void deleteLabel();
void figureCon();

#endif
