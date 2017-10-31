#ifndef PRASE_H
#define PRASE_H

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <memory.h>
#include <stdlib.h>
#include <stdarg.h>
#include <float.h>
#include "SyntaxTree.h"

/*
 * lex & parse
 */
extern int yylex(void);
extern int yyparse(void);
extern void yyrestart(FILE*);
extern void yyerror(char*);
extern int yylineno;

/*
 * SyntaxTree
 */
extern Node* tree;

#endif
