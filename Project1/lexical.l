%{
	#include <stdio.h>
	#include <stdlib.h>
	#include "SyntaxTree.h"
	#include "syntax.tab.h"

	int yycolumn = 1;
	/* define YY_USER_ACTION */
	#define YY_USER_ACTION \
		yylloc.first_line = yylloc.last_line = yylineno; \
		yylloc.first_column = yycolumn; \
		yylloc.last_column = yycolumn + yyleng - 1; \
		yycolumn += yyleng;

%}

%option yylineno

DELIM		[ \t]
WS		{DELIM}+
DIGIT		[0-9]
LETTER		[A-Za-z]
INT		0|([1-9]{DIGIT}*)
OCT		0([0-7]+)
HEX		0(x|X)[0-9a-fA-F]+
FLOAT		{INT}(\.{DIGIT}+)?
RELOP 		>|<|>=|<=|==|!=
ID		({LETTER}|_)({LETTER}|{DIGIT}|_)*
LINE		[\n]


%%


{WS}	{		}

";"		{ return SEMI;}

","		{ return COMMA;}

"="		{ return ASSIGNOP;}

{RELOP}	{ return RELOP;}

"+"		{ return PLUS;}

"-"		{ return MINUS;}

"*"		{ return STAR;}

"/"		{ return DIV;}

"&&"	{ return AND;}

"||"	{ return OR;}

"."		{ return DOT;}

"!"		{ return NOT;}

"("		{ return LP;}

")"		{ return RP;}

"["		{ return LB;}

"]"		{ return RB;}

"{"		{ return LC;}

"}"		{ return RC;}

"int"|"float"	{ return TYPE;}

"struct"	{ return STRUCT;}

"return"	{ return RETURN;}

"if"	{ return IF;}

"else"	{ return ELSE;}

"while"	{ return WHILE;}

{LINE}	{
			yycolumn = 1; 
		}

{INT}	{ return INT;}

{OCT}	{ return INT;}

{HEX}	{ return INT;}

{FLOAT}	{ return FLOAT;}

{ID}	{ return ID;}

. 		{ 
			printf("Error type A at line %d: Mysterious character \'%s\'\n"
			, yylineno, yytext); 
		}


%%

/*
int main(int argc, char** argv){
	if (argc > 1){
		if (!(yyin = fopen(argv[1], "r"))){
			perror(argv[1]);
			return 1;
		}
	}
	while (yylex() != 0) ;
	return 0;
}
*/
