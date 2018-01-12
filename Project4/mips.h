#ifndef MIPS_H
#define MIPS_H

#define REG_NUM 32
#define REG_T_START 8
#define REG_T_END 16
#define STR_LENGTH 200

#include "InterCode.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

typedef struct Var_t {
	char *name;
	int reg;
	int offset;
	struct Var_t *next;
} Var_t;

typedef struct Register_ {
	char *name;
	Var_t *var;
} Register_;

void printMips(char *fileName);
void printMipsCode(InterCode interCode);

void mipsLabel(InterCode interCode);
void mipsAssign(InterCode interCode);
void mipsOperation(InterCode interCodep);
void mipsRead(InterCode interCode);
void mipsWrite(InterCode interCode);
void mipsCall(InterCode interCode);
void mipsReturn(InterCode interCode);
void mipsGOTO(InterCode interCode);
void mipsIFGOTO(InterCode interCode);
void mipsFunction(InterCode interCode);
void mipsArg(InterCode interCode);
void mipsParam(InterCode interCode);
void mipsDec(InterCode interCode);

void initRegs();
int getReg(Operand op);
char* printReg(int index);
void swReg(int index);
void lwReg(int index, Var_t *var);

void delVars();
void addVar(Var_t *var);
Var_t* findVar(char *name);

extern Register_ regs[];
extern char* regName[];
extern FILE* fp;
extern Var_t *varList;
extern int curReg;
extern int spOffset;
extern int curParam;
extern int curArg;
extern const int stackSize;

#endif
