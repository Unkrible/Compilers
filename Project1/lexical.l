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


{WS}	{ /* tabs */ }

{INT}	{ 
			yylval.type_node = init("INT",yytext,TYPE_INT,yylineno);
			return INT;
		}

{OCT}	{ 
			yylval.type_node = init("INT",yytext,TYPE_OCT,yylineno);
			return INT;
		}

{HEX}	{ 
			yylval.type_node = init("INT",yytext,TYPE_HEX,yylineno);
			return INT;
		}

{FLOAT}	{ 
			yylval.type_node = init("FLOAT",yytext,TYPE_FLOAT,yylineno);
			return FLOAT;
		}

{ID}	{ 
			yylval.type_node = init("ID",yytext,TYPE_ID,yylineno);
			return ID;
		}

";"		{ 
			yylval.type_node = init("SEMI",yytext,TYPE_KEYWORD,yylineno);
			return SEMI;
		}

","		{ 
			yylval.type_node = init("COMMA",yytext,TYPE_KEYWORD,yylineno);
			return COMMA;
		}


"="		{ 
			yylval.type_node = init("ASSIGNOP",yytext,TYPE_KEYWORD,yylineno);
			return ASSIGNOP;
		}

{RELOP}	{ 
			yylval.type_node = init("RELOP",yytext,TYPE_KEYWORD,yylineno);
			return RELOP;
		}

"+"		{ 
			yylval.type_node = init("PLUS",yytext,TYPE_KEYWORD,yylineno);
			return PLUS;
		}

"-"		{ 
			yylval.type_node = init("MINUS",yytext,TYPE_KEYWORD,yylineno);
			return MINUS;
		}

"*"		{ 
			yylval.type_node = init("STAR",yytext,TYPE_KEYWORD,yylineno);
			return STAR;
		}

"/"		{ 
			yylval.type_node = init("DIV",yytext,TYPE_KEYWORD,yylineno);
			return DIV;
		}

"&&"	{ 
			yylval.type_node = init("AND",yytext,TYPE_KEYWORD,yylineno);
			return AND;
		}

"||"	{ 
			yylval.type_node = init("OR",yytext,TYPE_KEYWORD,yylineno);
			return OR;
		}

"!"		{ 
			yylval.type_node = init("NOT",yytext,TYPE_KEYWORD,yylineno);
			return NOT;
		}

"."		{
			yylval.type_node = init("DOT",yytext,TYPE_KEYWORD,yylineno);
			return DOT;
		}

"int"|"float"	{ 
					yylval.type_node = init("TYPE",yytext,TYPE_KEYWORD,yylineno);
					return TYPE;
				}

"("		{ 
			yylval.type_node = init("LP",yytext,TYPE_KEYWORD,yylineno);
			return LP;
		}

")"		{ 
			yylval.type_node = init("RP",yytext,TYPE_KEYWORD,yylineno);
			return RP;
		}

"["		{ 
			yylval.type_node = init("SEMI",yytext,TYPE_KEYWORD,yylineno);
			return LB;
		}

"]"		{ 
			yylval.type_node = init("RB",yytext,TYPE_KEYWORD,yylineno);
			return RB;
		}

"{"		{ 
			yylval.type_node = init("LC",yytext,TYPE_KEYWORD,yylineno);
			return LC;
		}

"}"		{ 
			yylval.type_node = init("RC",yytext,TYPE_KEYWORD,yylineno);
			return RC;
		}

"struct"	{ 
				yylval.type_node = init("STRUCT",yytext,TYPE_KEYWORD,yylineno);
				return STRUCT;
			}

"return"	{ 
				yylval.type_node = init("RETURN",yytext,TYPE_KEYWORD,yylineno);
				return RETURN;
			}

"if"	{ 
			yylval.type_node = init("IF",yytext,TYPE_KEYWORD,yylineno);
			return IF;
		}

"else"	{ 
			yylval.type_node = init("ELSE",yytext,TYPE_KEYWORD,yylineno);
			return ELSE;
		}

"while"	{ 
			yylval.type_node = init("WHILE",yytext,TYPE_KEYWORD,yylineno);
			return WHILE;
		}

{LINE}	{
			yycolumn = 1; 
		}


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
